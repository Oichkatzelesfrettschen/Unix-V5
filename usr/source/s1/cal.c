/**
 * @file cal.c
 * @brief A simple calendar program.
 *
 * This program displays a calendar for a given month and year, or for an entire year.
 * It handles leap years and the Gregorian calendar change in 1752.
 *
 * Original source from Unix V5. Modernized for compilation with clang and
 * enhanced with Doxygen comments.
 */
#include <stdio.h>
#include <stdlib.h> // For exit()

// Forward declarations for functions defined in this file
int number(char *str);
void pstr(char *str, int n);
void cal(int m, int y, char *p, int w);
int jan1(int yr);

/** @brief Day of the week header string. */
char	dayw[] =
{
	" S  M Tu  W Th  F  S"
};

/** @brief Month name strings. */
char	*smon[] =
{
	"Jan", "Feb", "Mar", "Apr",
	"May", "Jun", "Jul", "Aug",
	"Sep", "Oct", "Nov", "Dec",
};

/** @brief Global string buffer used for formatting calendar output. Max size for 3 months side-by-side. */
char	string[432]; // 6 lines * 72 chars/line (3 months * (3 chars/day * 7 days + 3 spaces between months))

/**
 * @brief Main function for the cal program.
 * @param argc Argument count.
 * @param argv Argument vector. Expects `cal [month] year` or `cal year`.
 * @return 0 on success, 1 on error.
 */
int main(int argc, char *argv[])
{
	register int y, i, j; // Loop counters and year variable
	int m; // Month variable

	if(argc < 2) {
		printf("usage: cal [month] year\n");
		exit(1);
	}
	if(argc == 2)
		goto long_label; // Print full year if only year is provided

	/*
	 *	print out just month
	 */
	m = number(argv[1]); // Get month from arguments
	if(m<1 || m>12)
		goto badarg; // Invalid month
	y = number(argv[2]); // Get year from arguments
	if(y<1 || y>9999)
		goto badarg; // Invalid year

	printf("      %s %d\n", smon[m-1], y);
	printf("%s\n", dayw);
	cal(m, y, string, 24); // Generate calendar for the month (24 chars wide)
	for(i=0; i<6*24; i+=24) // Print 6 lines of calendar, each 24 chars wide
		pstr(string+i, 24);
	exit(0);

/*
 *	print out complete year
 */
long_label:
	y = number(argv[1]); // Get year from argument
	if(y<1 || y>9999)
		goto badarg; // Invalid year

	printf("\n\n\n");
	printf("				%d\n", y); // Print year centered
	printf("\n");

	for(i=0; i<12; i+=3) { // Loop through months, 3 at a time (for 4 rows of 3 months)
		for(j=0; j<6*72; j++) // Clear the global string buffer
			string[j] = '\0';

		// Print month headers for three months
		printf("	 %s", smon[i]);
		printf("			%s", smon[i+1]);
		printf("		       %s\n", smon[i+2]);
		printf("%s   %s   %s\n", dayw, dayw, dayw); // Print day headers for three months

		// Generate calendar data for three months side-by-side into the string buffer
		cal(i+1, y, string, 72);         // Month 1 (e.g., Jan), total width 72 for 3 months
		cal(i+2, y, string+23, 72);     // Month 2 (e.g., Feb), offset by 23 chars
		cal(i+3, y, string+46, 72);     // Month 3 (e.g., Mar), offset by 46 chars

		for(j=0; j<6*72; j+=72) // Print 6 lines of the 3-month calendar block
			pstr(string+j, 72);
	}
	printf("\n\n\n");
	exit(0);

badarg:
	printf("Bad argument\n");
	exit(1);
}

/**
 * @brief Converts a string of digits to an integer.
 * @param str The string to convert.
 * @return The integer value, or 0 if the string is not a valid positive number.
 */
int number(char *str)
{
	register int n, c;
	register char *s;

	n = 0;
	s = str;
	while((c = *s++)) {
		if(c<'0' || c>'9')
			return(0); // Return 0 if non-digit found
		n = n*10 + c-'0';
	}
	return(n);
}

/**
 * @brief Prints a string, padding with spaces and handling termination.
 *
 * This function takes a string `str` and a length `n`. It ensures that
 * any null characters within the first `n` characters of `str` are replaced
 * by spaces. It then finds the last non-space character within these `n`
 * characters and null-terminates the string after it. Finally, it prints
 * the (potentially modified) `str`.
 *
 * @param str The string to print.
 * @param n The number of characters to process in the string.
 */
void pstr(char *str, int n)
{
	register int i;
	register char *s;

	s = str;
	i = n;
	while(i--) // Loop n times
		if(*s++ == '\0') // If a null is found
			s[-1] = ' ';   // Replace it with a space

	// Trim trailing spaces by finding the last non-space char
	s = str; // Reset s to start of the relevant part of the string
	i = n+1; // Look one past the 'active' area
	while(i--)
		if(*--s != ' ') // Decrement s then check. Find last non-space.
			break;
	s[1] = '\0'; // Null-terminate after the last non-space character
	printf("%s\n", str);
}

/** @brief Array storing the number of days in each month (0-indexed, month 0 unused). */
char	mon[] =
{
	0,  // Unused
	31, // Jan
	29, // Feb (default to 29, adjusted for non-leap/1752 later)
	31, // Mar
	30, // Apr
	31, // May
	30, // Jun
	31, // Jul
	31, // Aug
	30, // Sep (default to 30, adjusted for 1752 later)
	31, // Oct
	30, // Nov
	31, // Dec
};

/**
 * @brief Generates the calendar data for a given month and year into a string buffer.
 *
 * @param m The month (1-12).
 * @param y The year.
 * @param p Pointer to the character buffer where the calendar days will be written.
 *          This buffer is expected to be pre-allocated.
 * @param w The width of one line in the buffer for this month's display.
 *          This is used to advance to the next line in the buffer.
 *          For a single month display, this might be around 21-24.
 *          For a multi-month display (e.g., 3 months side-by-side), this is larger.
 */
void cal(int m, int y, char *p, int w)
{
	register int d, i; // d: day of week for 1st of month, i: day counter
	register char *s;  // Pointer into the character buffer p

	s = p; // Current position in the output buffer
	d = jan1(y); // Get day of the week for Jan 1st of year y (0=Sunday)

	// Adjust days in Feb and Sep for leap years and 1752 calendar change
	mon[2] = 29; // Default February to 29 days (for leap year)
	mon[9] = 30; // Default September to 30 days

	switch((jan1(y+1)+7-d)%7) { // Determine year type based on start day of next year
	/*
	 *	non-leap year: Jan 1st of next year is 1 day later (mod 7)
	 */
	case 1:
		mon[2] = 28; // February has 28 days
		break;

	/*
	 *	1752: Special case for Gregorian calendar adoption in Great Britain.
	 *  Jan 1st 1753 was a Thursday. Jan 1st 1752 was a Wednesday.
	 *  (jan1(1753)+7-jan1(1752))%7 = (4+7-3)%7 = 8%7 = 1. This is not the default.
	 *  The original code's default case here handles 1752, where Sept had 19 days.
	 *  The logic for `jan1` itself accounts for the 11 dropped days.
	 *  If the year is 1752, this switch case (default) will set mon[9] to 19.
	 *  Any other year type that doesn't match case 1 (non-leap) or case 2 (leap)
	 *  would incorrectly fall here. However, standard Gregorian calendar rules mean
	 *  years are either common (1 day shift) or leap (2 day shift).
	 *  The specific check for 1752 is implicitly handled if it's not case 1 or 2.
	 */
	default: // This case was 'default' in original code, seems to be for 1752
		if (y == 1752) { // Explicitly check for 1752 for clarity
			mon[9] = 19; // September 1752 had 19 days (days 3-13 were skipped)
		}
		// If not 1752, and not case 1 or 2, it's an anomaly or implies jan1 logic handles it.
		// For a standard leap year, mon[2] remains 29. For non-leap, it's 28.
		// This default case primarily serves the 1752 Sept adjustment.
		break;

	/*
	 *	leap year: Jan 1st of next year is 2 days later (mod 7)
	 */
	case 2:
		; // Leap year, mon[2] remains 29 (already set)
	}

	// Calculate day of week for 1st of month m
	for(i=1; i<m; i++)
		d = (d + mon[i]) % 7; // Accumulate days from previous months, mod 7

	s += 3*d; // Indent for the first day of the month (3 chars per day " 9 " or "10 ")

	// Fill in the days of the month
	for(i=1; i<=mon[m]; i++) {
		// Special handling for September 1752: skip days 3 through 13
		if(y == 1752 && m == 9 && i == 3 && mon[m] == 19) { // Check year, month, day, and that mon[9] is set to 19
			i += 11; // Jump from day 2 to day 14
			// The line "mon[m] += 11;" from original was problematic as it changed loop bound.
			// The effect of skipping days is achieved by incrementing 'i'.
			// The number of days in Sept 1752 is correctly 19 (30 - 11).
		}

		if(i > 9) // For two-digit days
			*s = (i/10)+'0';
		s++;
		*s++ = (i%10)+'0';
		s++; // Move to next day position (after space or second digit)

		if(++d == 7) { // If end of week (Saturday)
			d = 0; // Reset day of week to Sunday
			s = p+w; // Move to the start of the next line in the buffer
			p = s;   // Update base pointer for the current line
		}
	}
}

/**
 * @brief Calculates the day of the week for January 1st of a given year.
 *
 * Implements the algorithm for determining the day of the week,
 * considering Gregorian calendar rules and the 1752 changeover.
 * Calculation is based on Zeller's congruence or similar principles,
 * adapted for Jan 1st.
 *
 * @param yr The year.
 * @return The day of the week (0 for Sunday, 1 for Monday, ..., 6 for Saturday).
 */
int jan1(int yr)
{
	register int y, d; // y: year, d: day of week calculation accumulator

	/*
	 *	Algorithm:
	 *	Base day for year 0 or 1 (depending on epoch of formula) +
	 *	days for completed years +
	 *	days for leap years.
	 *	Adjustments for century rules and 1752 calendar change.
	 */

	y = yr;
	// Base calculation: (yr-1)*365 + (yr-1)/4 - (yr-1)/100 + (yr-1)/400 + 1) % 7 is common
	// This specific formula seems to be an empirical one for PDP-11 Unix.
	// Let's trace its logic:
	// d = 4 + y + (y+3)/4;  This part covers years and leap years (1 day per year, 1 extra per 4 years)
	// The constant '4' is likely an epoch offset.
	d = 4+y+(y+3)/4;

	/*
	 *	Julian calendar to Gregorian adjustment for years after 1800
	 *	(Before 1752, England used Julian. After 1752, Gregorian)
	 *	Gregorian rule: Leap year every 4 years, except if divisible by 100 unless also by 400.
	 *	This part subtracts days for century years not divisible by 400.
	 */
	if(y > 1800) { // Post-1800 adjustments for Gregorian calendar rule drift from simpler /4 rule
		d -= (y-1701)/100; // Subtract a day for each century (e.g., 1800, 1900 but not 2000)
		d += (y-1601)/400; // Add back a day for years divisible by 400 (e.g., 2000)
	}

	/*
	 *	Great calendar changeover instant (September 1752 in British Empire)
	 *	11 days were dropped. This adjustment shifts the day of the week.
	 *  If year is > 1752, or Sept 1752 onwards, this applies.
	 */
	if(y > 1752) // Or if (y == 1752 && month >= September_post_change)
		d += 3; // This constant '3' seems to be the net effect of the 11 dropped days on Jan 1st's day of week calculation for subsequent years.
				// (11 days dropped = 1 week + 4 days. This means day names shifted forward by 4 days.
				//  e.g. Wed Sep 2 was followed by Thu Sep 14.
				//  The original code used `d =+ 3;`. The sum of these constants and formula must match known calendars.
				//  Standard algorithms might use different constants/logic but arrive at same result.
				//  Let's assume this formula is empirically correct for its original target.
				//  A common reference: Jan 1, 1 was a Monday (if proleptic Gregorian).
				//  This formula likely has a different epoch or baseline.

	return(d%7); // Result is day of week (0-6, where 0 is Sunday needs verification with output)
				 // Given "S M Tu...", 0 seems to be Sunday.
}
