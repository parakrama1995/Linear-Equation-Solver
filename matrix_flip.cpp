#include <vector>
#include <iostream>
#include <fstream>
#include "BoolVector.h"

typedef int mybool;

using namespace std;


int main(int argc, char* argv[])
{
	//Open File
	ifstream inputFile("C:\\cygwin\\home\\David\\other\\crossdata");
	
	//construct arena
	std::vector< vector<char> > vvcArena;
	std::vector<mybool> vbRHS; //For the Matrix we'll have to solve
	char ctemp = inputFile.get();
    while (ctemp != '"')
        ctemp = inputFile.get();
    while (!inputFile.eof())
	{
		std::vector<char> vcRow;
		char c = inputFile.get();
        if (c == ';')
        {
            break;
        }
		while (c == '0' || c == '1' || c == '2' )
		{
			vcRow.push_back(c);

			if (c == '0')
				vbRHS.push_back(false);
			else if (c == '1')
				vbRHS.push_back(true);
			else if (c == '2')
				vbRHS.push_back(false);
			
			c = inputFile.get();
		}
		if (vcRow.size() > 0)
			vvcArena.push_back(vcRow);
	}

	//construct main matrix: initialize
	const int m = vvcArena.size();
	const int n = vvcArena[0].size();
	const int N = vbRHS.size(); //== m*n
	std::vector< BoolVector > vvbMain;
	for (int i = 0; i < N; i++)
	{
		BoolVector temp(N+1);
		if (vbRHS[i])
            temp.flipON(N);
		vvbMain.push_back(temp);
	}
	//construct main matrix: basis vectors
	for (int i = 0; i < m; i++)
	{
		for (int j = 0; j < n; j++)
		{
			vvbMain[i*n + j].flipON(i*n + j);
			if ( vvcArena[i][j] == '2' ) //Can't click a dud cell. 
				continue;

			int k = 0;
			while ( i-k > -1 && (vvcArena[i-k][j] != '2'))
				{vvbMain[(i-k)*n + j].flipON(i*n + j); k++;}
			k = 0;
			while ( i+k < m && (vvcArena[i+k][j] != '2'))
				{vvbMain[(i+k)*n + j].flipON(i*n + j); k++;}
			k = 0;
			while ( j-k > -1 && (vvcArena[i][j-k] != '2'))
				{vvbMain[(i)*n + (j-k)].flipON(i*n + j); k++;}
			k = 0;
			while ( j+k < n && (vvcArena[i][j+k] != '2'))
				{vvbMain[(i)*n + (j+k)].flipON(i*n + j); k++;}
		}
	}

	/*
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N+1; j++)
			cout << (int)vvbMain[i][j];
		cout << std::endl;
	}

	//*/

	rowReduceInZ_2(vvbMain);

	//*
	//Get solution back
	int p = 0;
	std::vector<mybool> vbAns;
	for (int i = 0; i < N; i++)
	{
		if (vvbMain[i-p][i])
			vbAns.push_back(vvbMain[i-p].back());
		else
		{
			vbAns.push_back(false);
			p++;
		}
	}


	cout << "<html>     <head> <script type=\"text/javascript\"> function post(){    var form = document.createElement('form');    form.action = \"http://www.hacker.org/cross/index.php\";    form.method = 'POST';    var input1 = document.createElement('input');    input1.type = 'hidden';    input1.name = 'name';    input1.value = 'klavierspieler21';    var input2 = document.createElement('input');    input2.type = 'hidden';    input2.name = 'password';    input2.value = 't7my2bE';    var input3 = document.createElement('input');    input3.type = 'hidden';    input3.name = 'sol';    input3.value = '";
	for (int i = 0; i < N; i++)
		cout << (int)vbAns[i];
    cout << "';    var input4 = document.createElement('input');    input4.type = 'hidden';    input4.name = 'go';    input4.value = 'submit';    form.appendChild(input1);    form.appendChild(input2);    form.appendChild(input3);    form.appendChild(input4);    form.submit()}post();</script> </head>  <body>     </body> </html>";
 
	//*/

	/*
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N+1; j++)
			cout << (int)vvbMain[i][j];
		cout << std::endl;
	}
	//*/


    return 0;
}
