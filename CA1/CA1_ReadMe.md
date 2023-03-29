# CA1: Quadratic Equation Solver Documentation

 ## Program Summary
 This program is a quadratic equation solver that takes the coefficients as input **string** and returns the roots as output **float**. The program will also indicate the discriminant of the equation and classify the types of roots acquired.

 ## User Guide
 1. key in any value in integer format for each coefficients a, b, c
 2. Upon entering the value for coefficient c, program will display the output result in a table format

 ## Program notes
 - Input accepts string integer (eg; "12") **REASON: Most coefficients are in integer type **
 - Whitespaces before and after the string integer will not crash the program & program will still run (eg; "     34  ")
 - Input does not accept float & program will request user to reinput the values for all the coefficient again
 - Program only accepts string integer where INT_MIN <= integer <= INT_MAX **REASON: Unlikely the coefficients will be too small or too large of a value **
 
## Test cases
- 2 real roots<br>
a = 5<br>
b = 10<br>
c = -3<br>

- 1 real root<br>
a = 1<br>
b = 6<br>
c = 9<br>
- Complex roots<br>
a = 2<br>
b = -6<br>
c = 5<br>




 