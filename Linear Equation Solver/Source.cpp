//=======================================================================
// Credits to William Hallahan
//=======================================================================
#pragma warning (disable : 4786)

#include <iostream>
#include <fstream>
#include <map>
#include "MatrixPackage.h"
#include "CharString.h"
#include "LinearEquationParser.h"

//======================================================================
//  Function Prototypes.
//======================================================================

void ReportParserError(const CharString & input_file_name_string,
                       int file_line,
                       int error_position,
                       const char * c_status_ptr);

void ReportProgramVersion();

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
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
    <Filter Include="Source Files">
      <UniqueIdentifier>{4FC737F1-C7A5-4376-A066-2A32D752A2FF}</UniqueIdentifier>
      <Extensions>cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx</Extensions>
    </Filter>
    <Filter Include="Header Files">
      <UniqueIdentifier>{93995380-89BD-4b04-88EB-625FBE52EBFB}</UniqueIdentifier>
      <Extensions>h;hh;hpp;hxx;hm;inl;inc;xsd</Extensions>
    </Filter>
    <Filter Include="Resource Files">
      <UniqueIdentifier>{67DA6AB6-F800-4c08-8B7A-83BB121AAD01}</UniqueIdentifier>
      <Extensions>rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx;tiff;tif;png;wav;mfcribbon-ms</Extensions>
    </Filter>
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="Source.cpp">
      <Filter>Source Files</Filter>
    </ClCompile>
  </ItemGroup>
</Project>
Contact GitHub API Training Shop Blog About

