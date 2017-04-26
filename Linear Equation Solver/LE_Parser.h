#ifndef LINEAREQUATIONPARSER_H
#define LINEAREQUATIONPARSER_H

#pragma warning (disable : 4786)

#include "MatrixPackage.h"
#include "CharString.h"

//======================================================================
//  Class Definition
//======================================================================

class LinearEquationParser
{
public:

    enum Status_T
    {
        SUCCESS,
        SUCCESS_NO_EQUATION,
        ERROR_ILLEGAL_EQUATION,
        ERROR_LAST_EQUATION_NOT_TERMINATED,
        ERROR_NO_EQUAL_SIGN,
        ERROR_MULTIPLE_EQUAL_SIGNS,
        ERROR_NO_TERM_BEFORE_EQUAL_SIGN,
        ERROR_NO_TERM_AFTER_EQUAL_SIGN,
        ERROR_NO_TERM_ENCOUNTERED,
        ERROR_NO_VARIABLE_IN_EQUATION,
        ERROR_MULTIPLE_DECIMAL_POINTS,
        ERROR_TOO_MANY_DIGITS,
        ERROR_MISSING_EXPONENT,
        ERROR_ILLEGAL_EXPONENT
    };

protected:

    int m_start_position;
    int m_error_position;
    Status_T m_last_error;
    int m_equation_index;
    int m_parser_state;
    bool m_negative_operator_flag;
    bool m_equal_sign_in_equation_flag;
    bool m_at_least_one_var_in_equation_flag;
    bool m_term_before_equal_sign_exists_flag;
    bool m_term_after_equal_sign_exists_flag;

public:

    typedef std::map<CharString, int> VariableNameIndexMap;

    LinearEquationParser();

    virtual ~LinearEquationParser();

    Status_T Parse(const CharString & input_line_string,
                   MatrixPackage::SparseMatrix & a_matrix,
                   MatrixPackage::SparseVector & b_vector,
                   VariableNameIndexMap & variable_name_index_map,
                   unsigned int & number_of_equations);

    void Reset();

    Status_T GetLastStatusValue() const;

    int GetErrorPosition() const;

    const char * GetStatusString(Status_T status);

protected:

    Status_T GetEquationStatus() const;

    void ResetForNewEquation();

    bool GetTerm(const CharString & input_line_string,
                 int & position,
                 MatrixPackage::SparseMatrix & a_matrix,
                 MatrixPackage::SparseVector & b_vector,
                 VariableNameIndexMap & variable_name_index_map);

    bool GetSign(const CharString & input_line_string,
                 int & position,
                 bool & negative_flag);

    bool GetNumber(const CharString & input_line_string,
                   int & position,
                   CharString & number_string);

    bool GetVariableName(const CharString & input_line_string,
                         int & position,
                         CharString & variable_name_string);

    bool GetOperator(const CharString & input_line_string,
                     int & position);

    void SkipSpaces(const CharString & input_line_string,
                    int & position);

    void SetLastStatusValue(Status_T last_error,
                            int error_position);
};

#endif
