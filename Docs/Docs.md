# NanoHS Docs

 - Variable declaration
 - Variable assignment
 - Auto casting variable declaration
  - If statement
 - While statement
 - Function declaration and return types

## Variable declaration
In NanoHS, there are three main ways to initialize any variable. A simple variable declaration, without assignin any value, is written like so:

    var:[type] [name];
Where [type] is the cast, [name] is the identifier that defines the variable.
[type] can be any of these (currently supported, could be expanded later): 

    double, int, string, bool, array, [struct name];

This declaration doesn't assign any value to the variable, but it limits it's content to the type it was initialized at the beginning, wich means that this is illegal:

    var:int DeclarationInt;
    DeclarationInt = "This is a string";

## Variable assignment
A value can be assigned to any variable in the usual manner:

    var:int decl; //Declaration
    decl = 7;
It's possible to assign a value on declaration:

    var:int decl = 7;	
  
  ## Automatic variable declaration
A variable can also be declared without it's type, as long as you immediatly assign it a value. From then on, it's type will be immutable.

    var [name] = [value];
    var autoDecl = 6; //it's type will be int from now on.//
    var autoDeclTwo = "String!"; //it's type will be string from now on.//
    var autoDeclThree = MyStruct(1,2,3, ...) //example with a struct.//
    var wrongAutoDecl; //Illegal! Will prompt a Parser Error.//

## If statement
An if statement can be prompted like this:

    if [condition(s)] {
	    [expression(s)];
	}
The condition can use the following operations:

    and, or, ==, !=, >=, <=, <, >, [custom functions]
**Important:** When using `and/or` it is important to separate them in parenthesis:

    if (a == 3) and ((4!=3) or (2>=3)) {
		//Do something
	} 
## While statement:
A while statement can be prompted like this:

    while [condition(s)] {
	    [expression(s)];
	}
The condition can use the following operations:

    and, or, ==, !=, >=, <=, <, >, [custom functions]
**Important:** When using `and/or` it is important to separate them in parenthesis:

    while (a == 3) and ((4!=3) or (2>=3)) {
		//Do something//
	} 

## Function  declaration
Functions can be declared like this:

    func [return types] name([var:[type] [name], ...){
	    [expression(s)];
	    return [return values];
    }
Multi return types must be incapsulated in parenthesis:

    func void FunctionVoid(){} //Void return function//
    func int FunctionInt(var:int Argument){} //Any single type return//
    func (int, double, ...) FunctionMulti(var:int Argument, var:double Second, ...){};
    
   A single/double return function can be written like this:
   
    func int CustomFuncOne(){
		//Do something//
		return intVariable;
	}
	
    func (int, double) CustomFunc(var:int argument){
		//Do something//
		return (intVariableOne, doubleVariableTwo);
	}

