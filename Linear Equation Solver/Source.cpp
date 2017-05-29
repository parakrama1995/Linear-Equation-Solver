#include <iostream>
#include <fstream>
#include <map>
#include "MatrixPackage.h"
#include "CharString.h"
#include "LinearEquationParser.h"

//======================================================================
//  Function Prototypes.
//======================================================================

void DisplayHelp();

#define MAXIMUM_INPUT_LINE_LENGTH (1024)
//#define DUMP_A_MATRIX_AND_B_VECTOR

//======================================================================
//  Start of main program.
//======================================================================

int main(int argc, char * argv[], char * envp[])
{
    //------------------------------------------------------------------
    //  Print the help prompt.
    //------------------------------------------------------------------

    if (argc <= 1)
    {
        ReportProgramVersion();
        std::cout << std::endl << "This program solves simultaneous linear equations that are stored in a file.";
        std::cout << std::endl;
        std::cout << std::endl << "For help type:";
        std::cout << std::endl;
        std::cout << std::endl << "        SolveLinearEquations -h";
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
        return 0;
    }

    //------------------------------------------------------------------
    //  Parse the command line.
    //------------------------------------------------------------------

    CharString input_file_name_string;
    bool display_program_name_flag = true;
    unsigned int input_file_name_count = 0;

    for (int i = 1; i < argc; i++)
    {
        if ((argv[i][0] == '-') || (argv[i][0] == '/'))
        {
            char c_control = (char)argv[i][1];

            switch (c_control)
            {
            //----------------------------------------------------------
            //  Display help information.
            //----------------------------------------------------------

            case 'h':
            case 'H':
            case '?':

                DisplayHelp();
                return 0;

            case 'q':
            case 'Q':

                display_program_name_flag = false;
                break;

            default:

                std::cout << "Illegal switch " << std::endl << argv[i] << std::endl;
                return 0;
            }
        }
        else
        {
            //----------------------------------------------------------
            //  Copy the file name to the input file name string
			//  variable.
            //----------------------------------------------------------

            input_file_name_count++;

            if (input_file_name_count == 1)
            {
                input_file_name_string = argv[i];
            }
            else
            {
                std::cout << "Illegal argument = " << argv[i] << std::endl;
                return 0;
            }
        }
    }

    //------------------------------------------------------------------
    //  Output the program name and the version string.
    //------------------------------------------------------------------

    if (display_program_name_flag)
    {
        ReportProgramVersion();
    }

    //------------------------------------------------------------------
    //  If an error occured in the input or the help was displayed then
    //  exit the program.
    //------------------------------------------------------------------

    if (input_file_name_string.IsEmpty())
    {
        std::cout << "No input file specified." << std::endl;
    }      
    else
    {
        //--------------------------------------------------------------
        //  If the input file has not extention then add ".txt" as the
        //  file extension.
        //--------------------------------------------------------------

        if (input_file_name_string.Find('.') == -1)
        {
            input_file_name_string += ".txt";
        }

        //--------------------------------------------------------------
        //  Open the input file.
        //--------------------------------------------------------------

        bool valid_system_of_equations_flag = true;

        std::ifstream input_file;
        input_file.open(input_file_name_string.CString(), std::ios::in);

        if (input_file.fail())
        {
            std::cout << "File " << input_file_name_string << " not found." << std::endl;
        }
        else
        {
            //----------------------------------------------------------
            //  Loop and read all lines from the input file.
            //----------------------------------------------------------

            LinearEquationParser equation_parser;
            LinearEquationParser::Status_T parser_status = LinearEquationParser::SUCCESS;
            MatrixPackage::SparseMatrix a_matrix;
            MatrixPackage::SparseVector b_vector;
            LinearEquationParser::VariableNameIndexMap variable_name_index_map;
            unsigned int number_of_equations = 0;
            int file_line = 0;

            while (!input_file.eof())
            {
                //------------------------------------------------------
                //  Read a line from the input file.
                //------------------------------------------------------

                const unsigned int maximum_line_length = MAXIMUM_INPUT_LINE_LENGTH;
                char input_data_array[maximum_line_length];

                input_file.getline(input_data_array,
                                   maximum_line_length);
                ++file_line;

                CharString input_line_string = input_data_array;

                //------------------------------------------------------
                //  If the line contains a comment delimiter which is
                //  the character sequence "//" then strip the delimiter
                //  and all characters that follow the delimiter.
                //------------------------------------------------------

                int comment_delimiter = input_line_string.Find("//");

                if (comment_delimiter != -1)
                {
                    input_line_string =
                        input_line_string.ExtractLeading(comment_delimiter);
                }

                //------------------------------------------------------
                //  Parse the line.
                //------------------------------------------------------

                parser_status =
                    equation_parser.Parse(input_line_string,
                                          a_matrix,
                                          b_vector,
                                          variable_name_index_map,
                                          number_of_equations);

                //------------------------------------------------------
                //  If a parser error occurs then report the error.
                //------------------------------------------------------

                valid_system_of_equations_flag =
                    parser_status == LinearEquationParser::SUCCESS;

                if (! valid_system_of_equations_flag)
                {
                    ReportParserError(input_file_name_string,
                                       file_line,
                                       equation_parser.GetErrorPosition(),
                                       equation_parser.GetStatusString(parser_status));
                    break;
                }
            }

            //----------------------------------------------------------
            //  Close the input file.
            //----------------------------------------------------------

            input_file.close();

#ifdef DUMP_A_MATRIX_AND_B_VECTOR
            //----------------------------------------------------------
            //  Dump the a_matrix and the b_vector.
            //----------------------------------------------------------

            int i = 0;
            for (i = 0; i < (int)(number_of_equations); ++i)
            {
                int j = 0;
                for (j = 0; j < (int)(number_of_equations); ++j)
                {
                    double value = a_matrix[DoubleIndex(i, j)];
                    std::cout << "a_matrix[" << i << ", " << j << "] = " << value << std::endl;
                }
            }

            for (i = 0; i < (int)(number_of_equations); ++i)
            {
                double value = b_vector[i];
                std::cout << "b_vector[" << i << "] = " << value << std::endl;
            }
#endif
            //----------------------------------------------------------
            //  Send a blank line to the parser to terminate the final
            //  equation and update the variable 'number_of_equations'.
            //----------------------------------------------------------

            if (valid_system_of_equations_flag)
            {
                CharString carriage_return_string = "\n";

                parser_status = equation_parser.Parse(carriage_return_string,
                                                       a_matrix,
                                                       b_vector,
                                                       variable_name_index_map,
                                                       number_of_equations);

                //------------------------------------------------------
                //  If a parser error occurs then report the error.
                //------------------------------------------------------

                valid_system_of_equations_flag =
                    parser_status == LinearEquationParser::SUCCESS;

                if (valid_system_of_equations_flag)
                {
                    //--------------------------------------------------
                    //  Test to make sure there are the same number of
                    //  equations as the number of variables.
                    //--------------------------------------------------

                    unsigned int number_of_variables = variable_name_index_map.size();

                    if (number_of_variables > number_of_equations)
                    {
                        std::cout << "There are " << number_of_variables
                            << " variables and only " << number_of_equations << " equations." << std::endl;
                    }
                    else if (number_of_variables < number_of_equations)
                    {
                        std::cout << "There are " << number_of_equations
                            << " equations and only " << number_of_variables << " variables." << std::endl;
                    }
                    else
                    {
                        MatrixPackage::SparseVector x_vector;

                        MatrixPackage::Status_T system_status =
                            MatrixPackage::SolveLinearEquations(number_of_equations,
                                                                 a_matrix,
                                                                 b_vector,
                                                                 x_vector);

                        if (system_status == MatrixPackage::SUCCESS)
                        {
                            //------------------------------------------
                            //  Display the solution of the equations.
                            //------------------------------------------

                            for (LinearEquationParser::VariableNameIndexMap::iterator it =
                                 variable_name_index_map.begin();
                                 it != variable_name_index_map.end();
                                 ++it)
                            {
                                std::cout << (*it).first << " = " << x_vector[(*it).second] << std::endl;
                            }
                        }
                        else
                        {
                            std::cout << MatrixPackage::GetStatusString(system_status) << std::endl;
                        }
                    }
                }
                else
                {
                    ReportParserError(input_file_name_string,
                                      file_line,
                                      equation_parser.GetErrorPosition(),
                                      equation_parser.GetStatusString(parser_status));
                }
            }
        }
    }

    return 0;
}

//======================================================================
//  Routine to report parser errors.
//======================================================================

void ReportParserError(const CharString & input_file_name_string,
                       int file_line,
                       int error_position,
                       const char * status_ptr)
{
    std::cout << "Error in input file " << input_file_name_string.CString()
        << " at line " << file_line << ", column "
        << error_position << "." << std::endl;

    std::cout << status_ptr << std::endl;

    return;
}

//======================================================================
//  Routine to report the program name and version number.
//======================================================================

void ReportProgramVersion()
{
    std::cout << "Linear Equation Solver - Version 2.01";
    std::cout << std::endl << "Copyright (C) William Hallahan 2001-2013.";
    std::cout << std::endl;
    return;
}

//======================================================================
//  Routine to report the program name and version number.
//======================================================================

void DisplayHelp()
{
    std::cout << std::endl << "\nThis program solves simultaneous linear equations that are stored";
    std::cout << std::endl << "in a file.";
    std::cout << std::endl;
    std::cout << std::endl << "Usage:";
    std::cout << std::endl;
    std::cout << std::endl << "        SolveLinearEquations -q Equations.txt";
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << std::endl << "The program takes a single file name as an argument. The file";
    std::cout << std::endl << "contains a set of simultaneous linear equations. If a file";
    std::cout << std::endl << "extension is specified then the file extension will defaults";
    std::cout << std::endl << "to the string \".txt\" .";
    std::cout << std::endl;
    std::cout << std::endl << "The -q switch suppresses program and version information.";
    std::cout << std::endl;
    std::cout << std::endl << "Comments can be included on any line in the file. The comments";
    std::cout << std::endl << "are started by the characters \"//\". All characters on the same";
    std::cout << std::endl << "line that occur after the comment characters are ignored.";
    std::cout << std::endl;
    std::cout << std::endl << "Each equation contains terms separated by either a plus sign character";
    std::cout << std::endl << "'+', a minus sign character '-', or an equal sign character '='.";
    std::cout << std::endl << "A term is a number, a variable, or a number followed by a variable.";
    std::cout << std::endl << "A number can be a floating-point number. The optional exponent";
    std::cout << std::endl << "of a floating point number  is preceded by the '^' character.";
    std::cout << std::endl << "A variable is composed of the letters from 'A' to 'Z' and 'a' to 'z'";
    std::cout << std::endl << "and the underscore '_' character.";
    std::cout << std::endl;
    std::cout << std::endl << "Equations can take a many lines as necessary. The only restriction";
    std::cout << std::endl << "is that a term cannot be split between lines. Equations are delimited";
    std::cout << std::endl << "by a semicolon. There is a " << MAXIMUM_INPUT_LINE_LENGTH << "character limit per line";
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << std::endl << "An example of a legal equation file is:";
    std::cout << std::endl;
    std::cout << std::endl << "  6Alpha+11=67;         // The semicolon ends the first equation";
    std::cout << std::endl << "  8 Beta + 23 = 4.5 - Alpha  // A blank line ends the second equation.";
    std::cout << std::endl;
    std::cout << std::endl << "  4.234^-05 + Gamma = 1000.0 Alpha + 2.0 Beta - 0.05";
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << std::endl << "Another example of a legal equation file is:";
    std::cout << std::endl;
    std::cout << std::endl << "  A = 1 ; B = 2 A; C = 2 B  // You can have more than one equation per line.";
    std::cout << std::endl;
    return;
}
#ifndef SPARSEARRAY_H
#define SPARSEARRAY_H

#include <map>

//------------------------------------------------------------------
//  Forward declarations.
//------------------------------------------------------------------

template <typename T_KEY, typename T_ITEM>

class SparseArray;

//------------------------------------------------------------------
//  Class definition for class SparseArrayProxy.
//
//  This is a proxy class for class SparseArray below.
//  See the book "More Effective C++" by Scott Meyers,
//  "Item 30: Proxy classes", page 213 for a detailed description
//  of proxy classes. The proxy class ideally is a nested class
//  inside the class that uses it, but many compilers do not
//  handle nested classes inside of templates correctly, so
//  the proxy is implemented as a stand-alone class.
//
//  In short, the proxy class allows determining whether
//  operator[]() for the SparseArray class is on the left side
//  or the right side of an equals sign in an expression.
//------------------------------------------------------------------

template <typename T_KEY, typename T_ITEM>

class SparseArrayProxy
{
public:

    SparseArrayProxy(SparseArray<T_KEY, T_ITEM> & sparse_array, T_KEY key);

    SparseArrayProxy & operator =(const SparseArrayProxy<T_KEY, T_ITEM> & right_hand_side);

    SparseArrayProxy & operator =(T_ITEM item);

    operator T_ITEM() const;

private:

    SparseArray<T_KEY, T_ITEM> & m_sparse_array;
    T_KEY m_key;
};

//------------------------------------------------------------------
//  Class definition for the sparse map.
//
//  Inheritance was used here, however, it might have been
//  cleaner to use containment for the STL map container and
//  to create methods to allow accessing the map from the
//  proxy class.
//------------------------------------------------------------------

template <typename T_KEY, typename T_ITEM>

class SparseArray : public std::map< T_KEY, T_ITEM, std::less<T_KEY> >
{
public:

    const SparseArrayProxy<T_KEY, T_ITEM> operator [](T_KEY key) const;

    SparseArrayProxy<T_KEY, T_ITEM> operator [](T_KEY key);
};

//----------------------------------------------------------------------
//  Implementation for methods of class SparseArrayProxy.
//
//  Constructor:
//
//    SparseArrayProxy(SparseArray & sparse_array, T_KEY key);
//----------------------------------------------------------------------

template <typename T_KEY, typename T_ITEM>

SparseArrayProxy<T_KEY, T_ITEM>::SparseArrayProxy(SparseArray<T_KEY, T_ITEM> & sparse_array,
                                                  T_KEY key)
  : m_sparse_array(sparse_array),
    m_key(key)
{
}

//----------------------------------------------------------------------
//  SparseArrayProxy & operator =(const SparseArrayProxy & right_hand_side);
//----------------------------------------------------------------------

template <typename T_KEY, typename T_ITEM>

SparseArrayProxy<T_KEY, T_ITEM> &
    SparseArrayProxy<T_KEY, T_ITEM>::operator =(const SparseArrayProxy<T_KEY, T_ITEM> & right_hand_side)
{
    //------------------------------------------------------------------
    //  If the item is the default item then clear the existing item
    //  at this key from the map.
    //------------------------------------------------------------------

    if (T_ITEM(right_hand_side) == T_ITEM())
    {
        typename SparseArray<T_KEY, T_ITEM>::iterator it = m_sparse_array.find(m_key);

        if (it != m_sparse_array.end())
        {
            m_sparse_array.erase(it);
        }
    }
    else
    {
        //--------------------------------------------------------------
        //  Add the item to the map at the specified key.
        //--------------------------------------------------------------

        (static_cast<std::map< T_KEY, T_ITEM, std::less<T_KEY> > &>(m_sparse_array))[m_key] = right_hand_side;
    }

    return *this;
}

//----------------------------------------------------------------------
//  SparseArrayProxy & operator =(T_ITEM & item);
//----------------------------------------------------------------------

template <typename T_KEY, typename T_ITEM>

SparseArrayProxy<T_KEY, T_ITEM> &
    SparseArrayProxy<T_KEY, T_ITEM>::operator =(T_ITEM item)
{
    (static_cast<std::map< T_KEY, T_ITEM, std::less<T_KEY> > &>(m_sparse_array))[m_key] = item;
    return *this;
}

//----------------------------------------------------------------------
//  operator T_ITEM() const;
//----------------------------------------------------------------------

template <typename T_KEY, typename T_ITEM>

SparseArrayProxy<T_KEY, T_ITEM>::operator T_ITEM() const
{
    typename SparseArray<T_KEY, T_ITEM>::iterator it = m_sparse_array.find(m_key);

    if (it != m_sparse_array.end())
    {
        return (*it).second;
    }
    else
    {
        return T_ITEM();
    }
}

//----------------------------------------------------------------------
//  Implementation for methods of class SparseArray.
//
//  const SparseArrayProxy operator [](T_KEY key) const;
//----------------------------------------------------------------------

template <typename T_KEY, typename T_ITEM>

const SparseArrayProxy<T_KEY, T_ITEM> SparseArray<T_KEY, T_ITEM>::operator [](T_KEY key) const
{
    return SparseArrayProxy<T_KEY, T_ITEM>(const_cast< SparseArray<T_KEY, T_ITEM> & >(*this), key);
}

//----------------------------------------------------------------------
//  SparseArrayProxy<T_KEY, T_ITEM> operator [](T_KEY key);
//----------------------------------------------------------------------

template <typename T_KEY, typename T_ITEM>

SparseArrayProxy<T_KEY, T_ITEM> SparseArray<T_KEY, T_ITEM>::operator [](T_KEY key)
{
    return SparseArrayProxy<T_KEY, T_ITEM>(*this, key);
}

#endif
MPI_Datatype xslice, yslice;
		MPI_Type_vector(block_step1 + 2, dim3, (block_step2 + 2) * dim3, MPI_DOUBLE, &xslice);
		MPI_Type_vector((block_step2 + 2) * dim3, 1, 1, MPI_DOUBLE, &yslice);
		MPI_Type_commit(&xslice);
		MPI_Type_commit(&yslice);
		if (rank == 0)
		{
			host_work(rank, size, MPI_GROUP_WORLD, row_group,
				coloum_group, row_comm, coloum_comm,
				xslice, yslice);
		}
		else
		{
			slave_work(rank, size, MPI_GROUP_WORLD, row_group,
				coloum_group, row_comm, coloum_comm,
				xslice, yslice);
		}
		MPI_Finalize();
		return 0;
void Input::splitInput()
{
	auto found = _input.begin();
	auto* current_container = &_left_expressions;
	while (found != _input.end())
	{
		// Switch container
		if (_input.front() == '=')
		{
			current_container = &_right_expressions;
			_input.erase(0, 1);
			prependPlus();
		}

		found = std::find_if(_input.begin() + 1, _input.end(), [&](const char& char_)
		{
			return isSeparatingOperator(char_);
		});

		current_container->emplace_back(_input.begin(), found);
		_input.erase(_input.begin(), found);
		found = _input.begin();
	}
}

// Put a + in front of the _input string if necessary
void Input::prependPlus()
{
	if (!isSeparatingOperator(_input.front()))
		_input.insert(0, 1, '+');
}

// Find out if character is separating operator in the front of the input (+/-)
bool Input::isSeparatingOperator(const char& char_)
{
	return (char_ == '+' || char_ == '-' || char_ == '=');
}

std::istream& operator>>(std::istream& is_, Input& obj_)
{
	// Create Shorthand
	auto& input = obj_._input;

	// Istream to string
	is_ >> input;

	// Remove spaces
	std::remove_if(input.begin(), input.end(), isspace);

	// Prepend + symbol
	obj_.prependPlus();

	// Split string to vectors
	obj_.splitInput();

	return is_;
}
void insertData(double **A, double *b, double *x)
{
	A[0][0] = 100.0; A[0][1] = -1.0; A[0][2] = 2.0; A[0][3] = -3.0;
	A[1][0] = 1.0; A[1][1] = 200.0; A[1][2] = -4.0; A[1][3] = 5.0;
	A[2][0] = -2.0; A[2][1] = 4.0; A[2][2] = 300.0; A[2][3] = -6.0;
	A[3][0] = 3.0; A[3][1] = -5.0; A[3][2] = 6.0; A[3][3] = 400.0;

	b[0] = 116.0; b[1] = -226.0; b[2] = 912.0; b[3] = -1174.0;

	x[0] = 2.0; x[1] = 2.0; x[2] = 2.0; x[3] = 2.0;
}

double est(double *x, double *x_nowe)
{
	x[0] = fabs(x[0] - x_nowe[0]);
	x[1] = fabs(x[1] - x_nowe[1]);
	x[2] = fabs(x[2] - x_nowe[2]);
	x[3] = fabs(x[3] - x_nowe[3]);

	double max = x[0];
	if (x[1] > max) max = x[1];
	if (x[2] > max) max = x[2];
	if (x[3] > max) max = x[3];

	return max;
}

double residuum(double **A, double *b, double *x_nowe)
{
	double Ax[n];

	Ax[0] = fabs((A[0][0] * x_nowe[0] + A[0][1] * x_nowe[1] + A[0][2] * x_nowe[2] + A[0][3] * x_nowe[3]) - b[0]);
	Ax[1] = fabs((A[1][0] * x_nowe[0] + A[1][1] * x_nowe[1] + A[1][2] * x_nowe[2] + A[1][3] * x_nowe[3]) - b[1]);
	Ax[2] = fabs((A[2][0] * x_nowe[0] + A[2][1] * x_nowe[1] + A[2][2] * x_nowe[2] + A[2][3] * x_nowe[3]) - b[2]);
	Ax[3] = fabs((A[3][0] * x_nowe[0] + A[3][1] * x_nowe[1] + A[3][2] * x_nowe[2] + A[3][3] * x_nowe[3]) - b[3]);

	double max = Ax[0];
	if (Ax[1] > max) max = Ax[1];
	if (Ax[2] > max) max = Ax[2];
	if (Ax[3] > max) max = Ax[3];

	return max;
}

void metoda_Jacobiego(double **A, double *b, double *x)
{
	double *x_nowe = new double[n]; //nowe przyblizenia
	double suma = 0.0;
	double EST = 0.0, RESIDUUM = 0.0;

	cout << endl << endl << "Metoda Jacobiego" << endl;
	cout << "  n |            x1 |            x2 |            x3 |            x4 |            EST |     RESIDIUM |" << endl;
	cout << "-----------------------------------------------------------------------------------------------------" << endl;

	for (int iter = 0; iter < il_petli; iter++)
	{
		for (int i = 0; i < n; i++)
		{
			suma = 0.0;
			for (int j = 0; j < n; j++)
			if (j != i)
				suma += A[i][j] * x[j];

			x_nowe[i] = (1.0 / A[i][i]) * (b[i] - suma);

		}

		EST = est(x, x_nowe);
		RESIDUUM = residuum(A, b, x_nowe);

		for (int i = 0; i < n; i++)
			x[i] = x_nowe[i];

		cout.width(4);
		cout << iter << "|";
		cout.width(15);
		cout << x_nowe[0] << "|";
		cout.width(15);
		cout << x_nowe[1] << "|";
		cout.width(15);
		cout << x_nowe[2] << "|";
		cout.width(15);
		cout << x_nowe[3] << "|";
		cout.width(15);
		cout << EST << "|";
		cout.width(15);
		cout << RESIDUUM << "|" << endl;

		if (EST < eps && RESIDUUM < eps)
			break;
	}
	cout << "-----------------------------------------------------------------------------------------------------" << endl;
}


void metoda_Gaussa_Seidela(double **A, double *b, double *x)
{
	double *x_poprz = new double[n]; //stare wart
	double suma = 0.0;
	double EST = 0.0, RESIDUUM = 0.0;

	cout << endl << endl << "\t Metoda Gaussa_Seidela" << endl;
	cout << "  n |            x1 |            x2 |            x3 |            x4 |            EST |     RESIDIUM |" << endl;
	cout << "-----------------------------------------------------------------------------------------------------" << endl;

	for (int iter = 0; iter < il_petli; iter++)
	{
		for (int i = 0; i < n; i++)
		{
			suma = 0.0;
			for (int j = 0; j < n; j++){
				if (j != i)
					suma += A[i][j] * x[j];
			}
			x_poprz[i] = x[i];
			x[i] = (1.0 / A[i][i]) * (b[i] - suma);
		}

		EST = est(x_poprz, x);
		RESIDUUM = residuum(A, b, x);


		cout.width(4);
		cout << iter << "|";
		cout.width(15);
		cout << x[0] << "|";
		cout.width(15);
		cout << x[1] << "|";
		cout.width(15);
		cout << x[2] << "|";
		cout.width(15);
		cout << x[3] << "|";
		cout.width(15);
		cout << EST << "|";
		cout.width(15);
		cout << RESIDUUM << "|" << endl;

		if (EST < eps&&RESIDUUM < eps)
			break;


	}
	cout << "-----------------------------------------------------------------------------------------------------" << endl;
}

void metoda_SOR(double **A, double *b, double *x)
{
	double *x_nowe = new double[n]; //nowe przyblizenia
	double *x_poprz = new double[n]; //stare wartosc
	double suma = 0.0, omega = 1.0;
	double EST = 0.0, RESIDUUM = 0.0;

	cout << endl << endl << "\t Metoda SOR" << endl;

	cout << "  n |            x1 |            x2 |            x3 |            x4 |            EST |     RESIDIUM |" << endl;
	cout << "-----------------------------------------------------------------------------------------------------" << endl;

	for (int iter = 0; iter < il_petli; iter++)
	{
		for (int i = 0; i < n; i++)
		{
			suma = 0.0;
			for (int j = 0; j < n; j++)
			if (j != i)
				suma += A[i][j] * x[j];

			x_poprz[i] = x[i];
			x_nowe[i] = (1.0 - omega) * x[i] + (omega / A[i][i]) * (b[i] - suma);
			x[i] = x_nowe[i];
		}

		EST = est(x_poprz, x_nowe);
		RESIDUUM = residuum(A, b, x_nowe);


		cout.width(4);
		cout << iter << "|";
		cout.width(15);
		cout << x[0] << "|";
		cout.width(15);
		cout << x[1] << "|";
		cout.width(15);
		cout << x[2] << "|";
		cout.width(15);
		cout << x[3] << "|";
		cout.width(15);
		cout << EST << "|";
		cout.width(15);
		cout << RESIDUUM << "|" << endl;

		if (EST < eps && RESIDUUM < eps)
			break;


	}
	cout << "-----------------------------------------------------------------------------------------------------" << endl;
}

void rozwiazanie()
{
	double **A, *b, *x;

	A = new double*[n];

	for (int i = 0; i < n; i++)
		A[i] = new double[n];

	b = new double[n];

	x = new double[n];

	insertData(A, b, x);

	cout << "Macierz A:" << endl;
	printMatrix(A);
	cout << endl;
	cout << " Wektor b:" << endl;
	printVector(b);
	cout << " Wektor x:" << endl;
	printVector(x);

	metoda_Jacobiego(A, b, x);
	system("pause");
	insertData(A, b, x);
	metoda_Gaussa_Seidela(A, b, x);
	system("pause");
	insertData(A, b, x);
	metoda_SOR(A, b, x);

	for (int i = 0; i < n; i++)
	{
		delete[] A[i];
	}

	delete[] A;
	delete[] b;
	delete[] x;

}
