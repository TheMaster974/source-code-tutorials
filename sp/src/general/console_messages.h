// ----------------------------------------------------------------------------------------
// Code written by Gareth Pugh, sourced from https://cplusplus.com/reference/cstdio/printf/
// ----------------------------------------------------------------------------------------
// The following code will demonstrate how to output a variable to the developer console.
// ----------------------------------------------------------------------------------------

#ifndef CONSOLE_MESSAGES_H
#define CONSOLE_MESSAGES_H

#include "cbase.h" // Needed for the Msg command to work.

// ------------------------------
// Here are some stock variables.
// ------------------------------
int negative = -8192; // Signed integer, can be negative. Can also use the keyword unsigned, which creates an integer that cannot be negative.
unsigned int decimal = 32; // Decimal integer, base 10.
unsigned int octal = 032; // Octaldecimal integer, base 8. 032 = 26 in decimal.
unsigned int hexa = 0x3B; // Hexadecimal integer, base 16. 0x3B = 59 in decimal.
float f = 420.69f; // Floating point number.
const char* string = "Sample text."; // Strings in the Source engine need to be defined as a const char* (most of the time).

// -------------------------------------------------------------------------------------------------
// If you want to use a macro that can be used for custom coloured messages, do something like this!
// -------------------------------------------------------------------------------------------------
// #define CustomMsg(...) ConColorMsg(Color(r, g, b, 255), "[Msg] " __VA_ARGS__ )

// ----------------------------------------------------------------------------
// Include this file somewhere by doing #include "console_messages.h" and
// call the function below to print some information about the above variables!
// ----------------------------------------------------------------------------
void PrintRandomVariables(void)
{
	ConVarRef developer("developer");
	if (developer.GetInt() < 1)
	{
		Warning("developer must be set to 1 or higher in order for this function to work properly!\n");
		return;
	}
// ----------------------------------------------------------------------
// This is how you would output these variables to the developer console.
// Msg can be used to print messages to the developer console, however
// you can be a little more creative with ConColorMsg/ConDColorMsg!
// ConColorMsg messages need developer set to 1 for them to be visible,
// ConDColorMsg messages need developer set to 2 for them to be visible.
// ----------------------------------------------------------------------
	Msg("==========================\n");
	Msg("Start of random variables!\n");
	Msg("==========================\n");

	Color randColor = Color(random->RandomInt(0, 255), random->RandomInt(0, 255), random->RandomInt(0, 255), 255); // Random RGB, 255 Alpha.
	ConColorMsg(randColor, "Signed integer = %i", negative); // Can use %i or %d.
	ConDColorMsg(randColor, " with RGB values of %i, %i, %i\n", randColor[0], randColor[1], randColor[2]);

	randColor = Color(random->RandomInt(0, 255), random->RandomInt(0, 255), random->RandomInt(0, 255), 255); // Reset for next message...
	ConColorMsg(randColor, "Unsigned integer = %u", decimal);
	ConDColorMsg(randColor, " with RGB values of %i, %i, %i\n", randColor[0], randColor[1], randColor[2]);

	randColor = Color(random->RandomInt(0, 255), random->RandomInt(0, 255), random->RandomInt(0, 255), 255); // ...And the next message...
	ConColorMsg(randColor, "Octal integer = %#o", octal);
	ConDColorMsg(randColor, " with RGB values of %i, %i, %i\n", randColor[0], randColor[1], randColor[2]);

	randColor = Color(random->RandomInt(0, 255), random->RandomInt(0, 255), random->RandomInt(0, 255), 255); // ...And the next message, etc.
	ConColorMsg(randColor, "Unsigned Hexadecimal integer = %#x", hexa); // Can use %x or %X.
	ConDColorMsg(randColor, " with RGB values of %i, %i, %i\n", randColor[0], randColor[1], randColor[2]);

	randColor = Color(random->RandomInt(0, 255), random->RandomInt(0, 255), random->RandomInt(0, 255), 255);
	ConColorMsg(randColor, "Floating point decimal = %f", f); // Can use %f or %F, %.<i>f to specify precision i.e. %.3f for 3 decimal places.
	ConDColorMsg(randColor, " with RGB values of %i, %i, %i\n", randColor[0], randColor[1], randColor[2]);

	randColor = Color(random->RandomInt(0, 255), random->RandomInt(0, 255), random->RandomInt(0, 255), 255);
	ConColorMsg(randColor, "Scientific notation = %e", f); // Can use %e or %E.
	ConDColorMsg(randColor, " with RGB values of %i, %i, %i\n", randColor[0], randColor[1], randColor[2]);

	randColor = Color(random->RandomInt(0, 255), random->RandomInt(0, 255), random->RandomInt(0, 255), 255);
	ConColorMsg(randColor, "Hexadecimal floating point = %a", f); // Can use %a or %A.
	ConDColorMsg(randColor, " with RGB values of %i, %i, %i\n", randColor[0], randColor[1], randColor[2]);

	randColor = Color(random->RandomInt(0, 255), random->RandomInt(0, 255), random->RandomInt(0, 255), 255);
	ConColorMsg(randColor, "Character = %c", string[0]); // Array indexing of a string to acquire a char.
	ConDColorMsg(randColor, " with RGB values of %i, %i, %i\n", randColor[0], randColor[1], randColor[2]);

	randColor = Color(random->RandomInt(0, 255), random->RandomInt(0, 255), random->RandomInt(0, 255), 255);
	ConColorMsg(randColor, "String = %s", string);
	ConDColorMsg(randColor, " with RGB values of %i, %i, %i\n", randColor[0], randColor[1], randColor[2]);

	randColor = Color(random->RandomInt(0, 255), random->RandomInt(0, 255), random->RandomInt(0, 255), 255);
	ConColorMsg(randColor, "Pointer address = %p and %p and %p and %p and %p and %p", negative, decimal, octal, hexa, f, string); // Memory address shenanigans!
	ConDColorMsg(randColor, " with RGB values of %i, %i, %i\n", randColor[0], randColor[1], randColor[2]);

	randColor = Color(random->RandomInt(0, 255), random->RandomInt(0, 255), random->RandomInt(0, 255), 255);
	ConColorMsg(randColor, "I'm feeling 100%% right around now!"); // Printing the % symbol to the developer console.
	ConDColorMsg(randColor, " with RGB values of %i, %i, %i\n", randColor[0], randColor[1], randColor[2]);

	Msg("==========================\n");
	Msg(" End of random variables! \n");
	Msg("==========================\n");

	/*
	Msg("Signed integer = %i\n", negative); // Can use %i or %d.
	Msg("Unsigned integer = %u\n", decimal);
	Msg("Octal integer = %#o\n", octal);
	Msg("Unsigned Hexadecimal integer = %#x\n", hexa); // Can use %x or %X.
	Msg("Floating point decimal = %f\n", f); // Can use %f or %F, %.<i>f to specify precision i.e. %.3f for 3 decimal places.
	Msg("Scientific notation = %e\n", f); // Can use %e or %E.
	Msg("Hexadecimal floating point = %a\n", f); // Can use %a or %A.
	Msg("Character = %c\n", string[0]); // Array indexing of a string to acquire a char.
	Msg("String = %s\n", string);
	Msg("Pointer address = %p\nand %p\nand %p\nand %p\nand %p\nand %p\n", negative, decimal, octal, hexa, f, string); // Memory address shenanigans!
	Msg("I'm feeling 100%% right around now!\n"); // Printing the % symbol to the developer console.
	*/
}

// You can also have length sub-specifiers, for example:
// %llu for an unsigned long long aka an unsigned 64-bit integer aka a uint64,
// or %# to put 0x/0 in front of a hexadecimal/octaldecimal value, respectively.
#endif // CONSOLE_MESSAGES_H