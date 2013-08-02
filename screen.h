/**
 * screen.h - Utility functions related to the screen.
 *
 * This file is part of lumail: http://lumail.org/
 *
 * Copyright (c) 2013 by Steve Kemp.  All rights reserved.
 *
 **
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 dated June, 1991, or (at your
 * option) any later version.
 *
 * On Debian GNU/Linux systems, the complete text of version 2 of the GNU
 * General Public License can be found in `/usr/share/common-licenses/GPL-2'
 */

#ifndef _screen_h_
#define _screen_h_ 1

#include <vector>
#include <unordered_map>
#include "maildir.h"

/**
 * This class contains simple functions relating to the screen-handling.
 */
class CScreen
{

public:

    /**
     * The segment of the screen the highlighted row is within.
     */
    enum vectorPosition
    {
        TOP,
        MIDDLE,
        BOTTOM,
        NONE
    };

    /**
     * Constructor.  NOP.
     */
    CScreen();

    /**
     * Destructor.  NOP.
     */
    ~CScreen();

    /**
     * Draw/Refresh the display.
     */
    void refresh_display();

    /**
     * Setup the screen.
     */
    void setup();

    /**
     * Return the width of the screen.
     */
    static int width();

    /**
     * Return the height of the screen.
     */
    static int height();

    /**
     * Clear the main display area, leaving the status-area alone.
     */
    static void clear_main();

    /**
     * Clear the status-line of the screen.
     */
    static void clear_status();

    /**
     * Choose a single item from a small selection.
     *
     * This will be used to resolve ambiguity in TAB-completion.
     */
    static std::string choose_string( std::vector<std::string> choices );

    /**
     * Handle TAB-expansion of an input string.
     *
     * NOTE: The caller must free the returned string.
     *
     * TODO: Return a std::vector when ambiguity is involved.
     */
    static char *get_completion( const char *input, size_t size, int position );

    /**
     * Get the possible completions for the current input line.
     */
    static std::vector<std::string> get_completions( const char *input, size_t size, int position );

    /**
     * Read a line of input.
     */
    static void readline( char *buffer, int buflen );

private:

    /**
     * Per-mode drawing primitives.
     */
    void drawMaildir();
    void drawIndex();
    void drawMessage();

    /**
     * Colour-maps.
     */
    std::unordered_map<std::string, int> m_colours;

};


#endif /* _screen_h_ */
