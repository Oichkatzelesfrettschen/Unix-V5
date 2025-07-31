cal - Calendar Utility
========================

The `cal` utility displays a calendar. It can show a calendar for a specific month and year, or for an entire year.

Functionality
-------------

*   Displays a single month: `cal [month] year`
*   Displays an entire year: `cal year`
*   Correctly handles leap years.
*   Accounts for the Gregorian calendar reform of 1752, where 11 days were omitted in September.

Source Code Documentation
-------------------------

The detailed documentation for the C source code of the `cal` utility, generated from Doxygen comments, is available below.

.. doxygenfile:: cal.c
   :project: UnixV5_Modernized

.. note::
   The `cal.c` source file is located at `usr/source/s1/cal.c` in the repository.
   The Doxygen comments have been added to the modernized version of this file.

Future Enhancements (Potential)
-------------------------------
*   Highlighting the current day.
*   Options for different week start days (e.g., Monday).
*   Color output.
