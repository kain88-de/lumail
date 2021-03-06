/**
 * bindings.cc - Bindings for all functions callable from Lua.
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
 *
 */


#include <stdio.h>
#include <algorithm>
#include <map>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <pcrecpp.h>
#include <ncurses.h>
#include <unistd.h>
#include <dirent.h>

#include "bindings.h"
#include "debug.h"
#include "file.h"
#include "maildir.h"
#include "lang.h"
#include "lua.h"
#include "global.h"
#include "screen.h"


int unused __attribute__((unused));


/**
 * Get/Set the value of a string variable.
 *
 * This function is called from all our lua-exposed functions that
 * get/set a simple string value.
 */
int get_set_string_variable( lua_State *L, const char * name )
{
    const char *str = lua_tostring(L, -1);
    CGlobal *g = CGlobal::Instance();

    if (str != NULL)
	g->set_variable( name, new std::string(str));

    std::string * s = g->get_variable(name);
    lua_pushstring(L, s->c_str());
    return( 1 );
}



/**
 * Get a message from the given path.
 *
 * If the path is NULL then we allocate the message from the heap as the
 * currently selected message.
 */
CMessage *get_message_for_operation( const char *path )
{
    CMessage *msg = NULL;

    /**
     * Given a path?  Use it.
     */
    if ( path != NULL )
    {
        msg = new CMessage( path );
        return(msg);
    }


    /**
     * OK we're working with the currently selected message.
     */

    /**
     * Get all messages from the currently selected messages.
     */
    CGlobal *global = CGlobal::Instance();
    std::vector<CMessage *> *messages = global->get_messages();

    /**
     * The number of items we've found, and the currently selected one.
     */
    int count    = messages->size();
    int selected = global->get_selected_message();

    /**
     * No messages?
     */
    if ( ( count < 1 ) || selected > count )
        return NULL;
    else
        return(  messages->at(selected) );

}



/**
 * Get the user's selected editor.
 */
std::string get_editor()
{
    /**
     * If this has been set use the editor
     */
    CGlobal *global = CGlobal::Instance();
    std::string *cmd  = global->get_variable("editor");
    if ( ( cmd != NULL ) && ( ! cmd->empty() ) )
        return( *cmd );

    /**
     * If there is an $EDITOR variable defined, use that.
     */
    std::string env = getenv( "EDITOR" );
    if ( !env.empty() )
        return( env );

    /**
     * Fall back to vim.
     */
    return( "vim" );

}



/**
 * Get, or set, the maildir-prefix
 */
int maildir_prefix(lua_State * L)
{
    /**
     * If we're setting it, make sure the value is sane.
     */
    const char *str = lua_tostring(L, -1);
    if (str != NULL)
    {
        if ( !CFile::is_directory( str ) )
            return luaL_error(L, "The specified prefix is not a Maildir" );
    }

    return( get_set_string_variable(L, "maildir_prefix" ) );
}


/**
 * Get, or set, the index-format
 */
int index_format(lua_State * L)
{
    return( get_set_string_variable(L, "index_format" ) );
}


/**
 * Get, or set, the editor
 */
int editor(lua_State * L)
{
    return( get_set_string_variable(L, "editor" ) );
}


/**
 * Get, or set, the message-filter.
 */
int message_filter(lua_State * L)
{
    return( get_set_string_variable(L, "message_filter" ) );
}


/**
 * Get, or set, the global lumail mode.
 */
int global_mode(lua_State * L)
{
    return( get_set_string_variable(L, "global_mode" ) );
}


/**
 * The format-string for maildir-display.
 */
int maildir_format(lua_State *L )
{
    return( get_set_string_variable( L, "maildir_format" ) );
}

/**
 * Get, or set, the maildir limit.
 */
int maildir_limit(lua_State * L)
{
    return( get_set_string_variable( L, "maildir_limit" ) );
}


/**
 * Get, or set, the index limit.
 */
int index_limit(lua_State * L)
{
    int ret =  get_set_string_variable( L, "index_limit" );

    /**
     * Update the selected mesages.
     */
    CGlobal *global = CGlobal::Instance();
    global->update_messages();
    return ret;
}


/**
 * Get, or set, the default from address.
 */
int from(lua_State * L)
{
    return( get_set_string_variable(L, "from" ) );
}

/**
 * Get, or set, the sendmail path.
 */
int sendmail_path(lua_State * L)
{
    return( get_set_string_variable( L, "sendmail_path" ) );
}


/**
 * Get, or set, the sent-folder path.
 */
int sent_mail(lua_State * L)
{
    return( get_set_string_variable( L, "sent_mail" ) );
}


/**
 * Colour support.
 */
int unread_message_colour(lua_State *L)
{
    return( get_set_string_variable( L, "unread_message_colour" ) );
}

int unread_maildir_colour(lua_State *L)
{
    return( get_set_string_variable( L, "unread_maildir_colour" ) );
}

int attachment_colour(lua_State *L)
{
    return( get_set_string_variable( L, "attachment_colour" ) );
}
int body_colour(lua_State *L)
{
    return( get_set_string_variable( L, "body_colour" ) );
}
int header_colour(lua_State *L)
{
    return( get_set_string_variable( L, "header_colour" ) );
}

/**
 * Clear the screen; but not the prompt.
 */
int clear(lua_State * L)
{
    CScreen::clear_main();
    return 0;
}


/**
 * Redraw the display.
 */
int refresh_display(lua_State * L)
{
    CScreen::clear_main();
    refresh();
    return 0;
}


/**
 * Sleep.
 */
int sleep(lua_State *L )
{
    int delay = lua_tointeger(L, -1);
    if (delay < 0 )
        return luaL_error(L, "positive integer expected for sleep(..)");

    sleep( delay );
    return 0;
}


/**
 * Exit the program.
 */
int exit(lua_State * L)
{
    endwin();

    CLua *lua = CLua::Instance();
    lua->call_function("on_exit");

    exit(0);
    return 0;
}


/**
 * Exit the program, abnormally.
 */
int abort(lua_State * L)
{
    const char *str = lua_tostring(L, -1);

    endwin();

    if (str != NULL)
        std::cerr << str << std::endl;

    exit(1);

    /**
     * Never reached.
     */
    return 0;
}


/**
 * Execute a program, resetting curses first.
 */
int exec(lua_State * L)
{
    const char *str = lua_tostring(L, -1);
    if (str == NULL)
	return luaL_error(L, "Missing argument to exec(..)");

    CScreen::clear_status();

    /**
     * Save the current state of the TTY
     */
    refresh();
    def_prog_mode();
    endwin();

    /* Run the command */
    unused = system(str);


    /**
     * Reset + redraw
     */
    reset_prog_mode();
    refresh();
    return 0;
}


/**
 * Write a message to the status-bar.
 */
int msg(lua_State * L)
{
    const char *str = lua_tostring(L, -1);

    if (str == NULL)
	return luaL_error(L, "Missing argument to msg(..)");

    CScreen::clear_status();
    move(CScreen::height() - 1, 0);
    printw("%s", str);
    return 0;
}


/**
 * Prompt for input.
 */
int prompt(lua_State * L)
{
    /**
     * Get the prompt string.
     */
    const char *str = lua_tostring(L, -1);
    if (str == NULL)
	return luaL_error(L, "Missing argument to prompt(..)");

    char input[1024] = { '\0' };
    curs_set(1);
    echo();

    CScreen::clear_status();
    move(CScreen::height() - 1, 0);
    printw(str);

    timeout(-1000);
    CScreen::readline( input, sizeof(input));
    noecho();
    timeout(1000);

    curs_set(0);

    CScreen::clear_status();
    lua_pushstring(L, strdup(input));
    return 1;
}


/**
 * Prompt for "y/n".
 */
int prompt_yn(lua_State * L)
{
    const char *def_prompt = "y/n?>";

    /**
     * Get the prompt string.
     */
    const char *str = lua_tostring(L, -1);
    if (str == NULL)
	str = def_prompt;

    /**
     * Push the characters and the prompt.
     */
    lua_pushstring(L, str);
    lua_pushstring(L,"ynYN");

    /**
     * Now we need to remap the return value:
     *
     *   y/Y -> true
     *   n/N -> false
     */
    if ( prompt_chars( L ) == 1 )
    {
        const char *response = lua_tostring(L,-1);
        if ( response != NULL &&
             ( response[0] == 'y' || response[0] == 'Y' ) )
        {
            lua_pushinteger(L, 1 );
            return 1;
        }
        if ( response != NULL &&
             ( response[0] == 'n' || response[0] == 'N' ) )
        {
            lua_pushinteger(L, 0 );
            return 1;
        }
    }
                 return( 0 );
}


/**
 * Prompt for a single character from the set.
 */
int prompt_chars(lua_State *L)
{
    /**
     * Get the prompt string, and response set.
     */
    const char *str   = lua_tostring(L, -2);
    const char *chars = lua_tostring(L, -1);

    /**
     * Ensure both were set.
     */
    if (str == NULL)
	return luaL_error(L, "Missing prompt to function prompt_chars(..)");
    if (chars == NULL)
	return luaL_error(L, "Missing characters to function prompt_chars(..)");


    echo();

    int height = CScreen::height();
    timeout(-1000);

    bool done = true;

    while (done)
    {
        /**
         * Refresh the prompt.
         */
        CScreen::clear_status();
        move(height - 1, 0);
        printw(str);

	char key = getch();

        /**
         * See if the character was in the input string.
         */
        for( unsigned int i = 0; i < strlen(chars); i++ )
        {
            if ( chars[i] == key )
            {
                char resp[2] = { '\0' };
                resp[0] = key;
                resp[1] = '\0';
                lua_pushstring(L, resp );
                done = false;
            }
        }
    }
    noecho();
    curs_set(0);
    timeout(1000);

    CScreen::clear_status();
    return 1;
}


/**
 * Prompt for a maildir.
 */
int prompt_maildir(lua_State * L)
{
    CGlobal *global = CGlobal::Instance();
    int selected = 0;
    int height = CScreen::height();


    while (true)
    {
        CScreen::clear_main();

        std::vector<CMaildir> folders = global->get_all_folders();

        int count = folders.size();
        if ( count < 1 )
        {
            lua_pushnil(L);
            return 1;
        }

        if ( selected > count )
            selected = count;
        if ( selected < 0 )
            selected = 0;


        /**
         * Current selection
         */
        CMaildir current = folders.at(selected);

        move(0,0);
        printw("Select a folder:");

        for (int row = 0; row < (height - 3); row++)
        {
            CMaildir *cur = NULL;
            if ((row + selected) < count)
            {
                cur = &folders[row + selected];

                move( row+2, 0 );
                printw( "%s", cur->path().c_str() );
            }
        }

	char key = getch();
	if (key == ERR)
        {
            // NOP
        }
        if ( key == 'j' )
            selected += 1;
        if ( key == 'k' )
            selected -= 1;
        if ( key == '\n' )
        {
            lua_pushstring(L, current.path().c_str() );
            return 1;
        }
    }

    return 0;
}

/**
 * scroll up/down the maildir list.
 */
int scroll_maildir_down(lua_State * L)
{
    int step = lua_tonumber(L, -1);

    CGlobal *global = CGlobal::Instance();

    int cur = global->get_selected_folder();
    cur += step;

    global->set_selected_folder(cur);

    return 0;
}


/**
 * Scroll the maildir list up.
 */
int scroll_maildir_up(lua_State * L)
{
    int step = lua_tonumber(L, -1);

    CGlobal *global = CGlobal::Instance();
    int cur = global->get_selected_folder();
    cur -= step;

    if (cur < 0)
	cur = 0;

    global->set_selected_folder(cur);

    return (0);
}


/**
 * Jump to the given entry in the maildir list.
 */
int jump_maildir_to(lua_State * L)
{
    int offset = lua_tonumber(L, -1);
    CGlobal *global = CGlobal::Instance();
    global->set_selected_folder(offset);

    return 0;
}


/**
 * scroll up/down the message list.
 */
int scroll_index_down(lua_State * L)
{
    int step = lua_tonumber(L, -1);

    CGlobal *global = CGlobal::Instance();

    int cur = global->get_selected_message();
    cur += step;

    global->set_selected_message(cur);

    /**
     * We've changed messages, so reset the current position.
     */
    global->set_message_offset(0);

    return 0;
}


/**
 * Scroll the index list up.
 */
int scroll_index_up(lua_State * L)
{
    int step = lua_tonumber(L, -1);

    CGlobal *global = CGlobal::Instance();
    int cur = global->get_selected_message();
    cur -= step;

    if (cur < 0)
	cur = 0;

    global->set_selected_message(cur);

    /**
     * We've changed messages, so reset the current position.
     */
    global->set_message_offset(0);

    return (0);
}


/**
 * Scroll the message down.
 */
int scroll_message_down(lua_State *L)
{
    int step = lua_tonumber(L, -1);

    CGlobal *global = CGlobal::Instance();
    int cur = global->get_message_offset();
    cur += step;

    global->set_message_offset(cur);
    return (0);
}

/**
 * Scroll the message up.
 */
int scroll_message_up(lua_State *L)
{
    int step = lua_tonumber(L, -1);

    CGlobal *global = CGlobal::Instance();
    int cur = global->get_message_offset();
    cur -= step;

    if ( cur < 0 )
        cur = 0;

    global->set_message_offset(cur);
    return (0);
}


/**
 * Jump to the given message.
 */
int jump_index_to(lua_State * L)
{
    int offset = lua_tonumber(L, -1);

    CGlobal *global = CGlobal::Instance();
    global->set_selected_message(offset);

    /**
     * We've changed messages, so reset the current position.
     */
    global->set_message_offset(0);

    return (0);
}


/**
 * scroll to the folder matching the pattern.
 */
int scroll_maildir_to(lua_State * L)
{
    const char *str = lua_tostring(L, -1);

    if (str == NULL)
	return luaL_error(L, "Missing argument to scroll_maildir_to(..)");


    /**
     * Lower-case version of the string.
     */
    std::string find(str);
    std::transform(find.begin(), find.end(), find.begin(), tolower);

    /**
     * get the current folders.
     */
    CGlobal *global = CGlobal::Instance();
    std::vector < CMaildir > display = global->get_folders();
    int max = display.size();
    int selected = global->get_selected_folder();

    int i = selected + 1;

    while (i != selected)
    {
	if (i >= max)
	    break;

        /**
         * Get the display-name of the folder.
         */
	CMaildir cur = display[i];
        std::string p = cur.path();

        /**
         * Lower-case it.
         */
        std::transform(p.begin(), p.end(), p.begin(), tolower);

	if (strstr(p.c_str(), find.c_str()) != NULL)
        {
	    global->set_selected_folder(i);
	    break;
	}
	i += 1;

	if (i >= max)
	    i = 0;
    }
    return 0;
}


/**
 * Get the currently highlighted maildir folder.
 */
int current_maildir(lua_State * L)
{
    /**
     * get the current folders.
     */
    CGlobal *global = CGlobal::Instance();
    std::vector < CMaildir > display = global->get_folders();
    int selected = global->get_selected_folder();

    CMaildir x = display[selected];
    lua_pushstring(L, x.path().c_str());
    return 1;
}


/**
 * Select a maildir by the path.
 */
int select_maildir(lua_State *L)
{
    const char *path = lua_tostring(L, -1);
    if (path == NULL)
	return luaL_error(L, "Missing argument to select_maildir(..)");


    /**
     * get the current folders.
     */
    CGlobal *global = CGlobal::Instance();
    std::vector < CMaildir > display = global->get_folders();
    int count = display.size();

    /**
     * Iterate over each one, and if we have a match then
     * set the selected one, and return true.
     */
    int i = 0;

    for( i = 0; i < count; i++ )
    {
        CMaildir cur = display[i];

        if ( strcmp( cur.path().c_str(), path ) == 0 )
        {
            /**
             * The current folder has the correct
             * path, so we'll select it.
             */
            global->set_selected_folder( i );

            /**
             * Return: true
             */
            lua_pushboolean(L,1);
            return 1;

        }
    }

    /**
     * Return: false
     */
    lua_pushboolean(L,0);
    return 1;
}


/**
 * Return maildirs matching a given pattern.
 */
int maildirs_matching(lua_State *L)
{
    const char *pattern = lua_tostring(L, -1);
    if (pattern == NULL)
	return luaL_error(L, "Missing argument to maildirs_matching(..)");

    /**
     * Get all maildirs.
     */
    CGlobal *global = CGlobal::Instance();
    std::vector<CMaildir> folders = global->get_all_folders();
    std::vector<CMaildir>::iterator it;

    /**
     * create a new table.
     */
    lua_newtable(L);

    /**
     * Lua indexes start at one.
     */
    int i = 1;

    /**
     * We need to use a pointer to string for our
     * testing-function.
     */
    std::string *filter  = new std::string( pattern );

    /**
     * For each maildir - add it to the table if it matches.
     */
    for (it = folders.begin(); it != folders.end(); ++it)
    {
        CMaildir f = (*it);


        if ( f.matches_filter( filter ) )
        {
            lua_pushnumber(L,i);
            lua_pushstring(L,(*it).path().c_str());
            lua_settable(L,-3);
            i++;
        }
    }

    /**
     * Avoid leaking the filter.
     */
    delete( filter );

    return 1;
}


/**
 * Count the visible maildir folders.
 */
int count_maildirs(lua_State *L)
{
    CGlobal *global = CGlobal::Instance();

    std::vector<CMaildir> folders = global->get_folders();
    lua_pushinteger(L, folders.size() );
    return 1;
}

/**
 * Get the names of all currently visible maildirs
 */
int current_maildirs(lua_State *L)
{
    CGlobal *global = CGlobal::Instance();
    std::vector < CMaildir > display = global->get_folders();
    std::vector<CMaildir>::iterator it;

    /**
     * Create the table.
     */
    lua_newtable(L);

    int i = 1;
    for (it = display.begin(); it != display.end(); ++it)
    {
        lua_pushnumber(L,i);
        lua_pushstring(L,(*it).path().c_str());
        lua_settable(L,-3);
        i++;
    }

    return 1;
}

/**
 * Get the currently highlighted message-path.
 */
int current_message(lua_State * L)
{
    /**
     * Get the currently selected message.
     */
    CMessage *msg = get_message_for_operation( NULL );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * If that succeeded store the path.
     */
    std::string source = msg->path();
    if ( !source.empty() )
    {
        lua_pushstring(L, source.c_str());
        return(1);
    }
    else
    {
        return 0;
    }
}





/**
 * Is the named/current message new?
 */
int is_new(lua_State * L)
{
    /**
     * Get the path (optional).
     */
    const char *str = lua_tostring(L, -1);
    int ret = 0;

    CMessage *msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }
    else
    {
        if ( msg->is_new() )
            lua_pushboolean(L,1);
        else
            lua_pushboolean(L,0);

        ret = 1;
    }

    if ( str != NULL )
        delete( msg );

    return( ret );
}


/**
 * Mark the message as read.
 */
int mark_read(lua_State * L)
{
    /**
     * Get the path (optional).
     */
    const char *str = lua_tostring(L, -1);

    CMessage *msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }
    else
        msg->mark_read();

    if ( str != NULL )
        delete( msg );

    return( 0 );
}


/**
 * Get a header from the current/specified message.
 */
int header(lua_State * L)
{
    /**
     * Get the path (optional), and the header (required)
     */
    const char *header = lua_tostring(L, 1);
    const char *path   = lua_tostring(L, 2);
    if ( header == NULL )
        return luaL_error(L, "Missing header" );

    /**
     * Get the message
     */
    CMessage *msg = get_message_for_operation( path );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Get the header.
     */
    std::string value = msg->header( header );
    lua_pushstring(L, value.c_str() );


    if ( path != NULL )
        delete( msg );

    return( 1 );
}


/**
 * Mark the message as new.
 */
int mark_new(lua_State * L)
{
    /**
     * Get the path (optional).
     */
    const char *str = lua_tostring(L, -1);

    CMessage *msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }
    else
        msg->mark_new();

    if ( str != NULL )
        delete( msg );

    return( 0 );
}


/**
 * Delete a message.
 */
int delete_message( lua_State *L )
{
    /**
     * Get the path (optional).
     */
    const char *str = lua_tostring(L, -1);

    CMessage *msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }
    else
    {
        unlink( msg->path().c_str() );

        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"Deleted: " + msg->path() + "\");" );
    }

    /**
     * Free the message.
     */
    if ( str != NULL )
        delete( msg );

    /**
     * Update messages
     */
    CGlobal *global = CGlobal::Instance();
    global->update_messages();

    /**
     * We're done.
     */
    return 0;
}


/**
 * Save the current message to a new location.
 */
int save_message( lua_State *L )
{
    const char *str = lua_tostring(L, -1);

    if (str == NULL)
	return luaL_error(L, "Missing argument to save(..)");

    if ( !CFile::is_directory( str ) )
        return luaL_error(L, "The specified destination is not a Maildir" );

    /**
     * Get the message
     */
    CMessage *msg = get_message_for_operation( NULL );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Got a message ?
     */
    std::string source = msg->path();

    /**
     * The new path.
     */
    std::string dest = CMaildir::message_in( str, ( msg->is_new() ) );

    /**
     * Copy from source to destination.
     */
    CFile::copy( source, dest );

    /**
     * Remove source.
     */
    unlink( source.c_str() );

    /**
     * Update messages
     */
    CGlobal *global = CGlobal::Instance();
    global->update_messages();

    /**
     * We're done.
     */
    return 0;
}


/**
 * Search for the next message matching the pattern.
 */
int scroll_index_to(lua_State * L)
{
    const char *str = lua_tostring(L, -1);

    if (str == NULL)
        return luaL_error(L, "Missing argument to scroll_index_to(..)");

    /**
     * Lower-case version of the string.
     */
    std::string find(str);
    std::transform(find.begin(), find.end(), find.begin(), tolower);

    /**
     * get the current messages
     */
    CGlobal *global = CGlobal::Instance();
    std::vector<CMessage *> *messages = global->get_messages();

    /**
     * If we have no messages we're not scrolling anywhere.
     */
    if ( messages == NULL )
        return 0;


    int max      = messages->size();
    int selected = global->get_selected_message();

    int i = selected + 1;

    while (i != selected) {
        if (i >= max)
            break;

        /**
         * Format the message, and lower-case it.
         */
        CMessage *cur = messages->at(i);
        std::string fmt = cur->format();
        std::transform(fmt.begin(), fmt.end(), fmt.begin(), tolower);

        /**
         * Now look for it.
         */
        if (strstr(fmt.c_str(), find.c_str()) != NULL)
        {
            global->set_selected_message(i);
            break;
        }
        i += 1;

        if (i >= max)
            i = 0;
    }

    /**
     * We've changed messages, so reset the current position.
     */
    global->set_message_offset(0);
    return 0;
}


/**
 * Get the currently selected folders.
 */
int selected_folders(lua_State * L)
{
    CGlobal *global = CGlobal::Instance();
    std::vector<std::string> selected = global->get_selected_folders();
    std::vector<std::string>::iterator it;

    /**
     * Create the table.
     */
    lua_newtable(L);

    int i = 1;
    for (it = selected.begin(); it != selected.end(); ++it)
    {
        lua_pushnumber(L,i);
        lua_pushstring(L,(*it).c_str());
        lua_settable(L,-3);
        i++;
    }

    return 1;
}


/**
 * Clear all currently selected folders.
 */
int clear_selected_folders(lua_State * L)
{
    CGlobal *global = CGlobal::Instance();
    global->unset_folders();
    global->set_selected_message(0);
    global->update_messages();

    /**
     * Call our update with an empty path.
     */
    CLua *lua = CLua::Instance();
    lua->execute("on_folder_selection(\"\");");

    return 0;
}


/**
 * Add the given folder to the selected set.
 */
int add_selected_folder(lua_State * L)
{
    /**
     * get the optional argument.
     */
    const char *str = lua_tostring(L, -1);

    CGlobal *global = CGlobal::Instance();
    CLua    *lua    = CLua::Instance();

    /**
     * The path that is being added.
     */
    std::string path;

    /**
     * default to the current folder.
     */
    if (str == NULL)
    {
	int selected = global->get_selected_folder();
	std::vector < CMaildir > display = global->get_folders();

        if ( display.size()  == 0 )
            return 0;

	CMaildir x = display[selected];
        path = x.path();
        global->add_folder(path.c_str());
    }
    else
    {
        path = std::string(str);
	global->add_folder(path);
    }

    global->set_selected_message(0);
    global->update_messages();

    if ( ! path.empty() )
        lua->execute("on_folder_selection(\"" + path + "\");");

    return (0);
}


/**
 * Remove all currently selected folders.  Add single new one.
 */
int set_selected_folder(lua_State * L)
{
    /**
     * get the optional argument.
     */
    const char *str = lua_tostring(L, -1);

    CGlobal *global = CGlobal::Instance();
    global->unset_folders();

    /**
     * The path we're adding.
     */
    std::string path;

    /**
     * default to the current folder.
     */
    if (str == NULL)
    {
	std::vector < CMaildir > display = global->get_folders();
	int selected = global->get_selected_folder();

	CMaildir x = display[selected];
        path = x.path();
	global->add_folder(path.c_str());
    }
    else
    {
        path = std::string(str);
	global->add_folder(path.c_str());
    }

    global->update_messages();

    if ( ! path.empty() )
    {
        CLua *lua = CLua::Instance();
        lua->execute("on_folder_selection(\"" + path + "\");");
    }

    return (0);
}



/**
 * Toggle the selection state of the currently selected folder.
 */
int toggle_selected_folder(lua_State * L)
{
    /**
     * get the optional argument.
     */
    const char *str = lua_tostring(L, -1);
    CGlobal *global = CGlobal::Instance();
    std::vector < std::string > sfolders = global->get_selected_folders();

    /**
     * default to the current folder.
     */
    std::string toggle;

    if (str == NULL)
    {
	std::vector < CMaildir > display = global->get_folders();
        if ( display.size()  == 0 )
            return 0;

	int selected = global->get_selected_folder();
	CMaildir x = display[selected];
	toggle = x.path();
    }
    else
    {
	toggle = std::string(str);
    }

    if (std::find(sfolders.begin(), sfolders.end(), toggle) != sfolders.end())
	global->remove_folder(toggle);
    else
        global->add_folder(toggle);

    global->update_messages();

    if ( ! toggle.empty() )
    {
        CLua *lua = CLua::Instance();
        lua->execute("on_folder_selection(\"" + toggle + "\");");
    }
    return (0);
}


/**
 * Get the MIME-type of a file, based upon suffix.
 *
 * We use /etc/mime.types to do this properly.
 */
int mime_type(lua_State *L)
{
    /**
     * Map of file-suffix to MIME-types.
     */
    static std::map< std::string, std::string> table;


    /**
     * Default MIME-type if we can't find the real one.
     */
    const char *default_type = "application/octet-stream";


    /**
     * Get the file to test.
     */
    const char *file = lua_tostring(L, -1);
    if (file == NULL)
	return luaL_error(L, "Missing argument to mime_type(..)");

    /**
     * Attempt to find the extension.
     */
    std::string filename(file);
    size_t offset = filename.rfind('.');

    /**
     * If we found one.
     */
    if(offset != std::string::npos)
    {
        /**
         * Get the lower-case version.
         */
        std::string extension = filename.substr(offset+1);
        std::transform(extension.begin(), extension.end(), extension.begin(), tolower);


        /**
         * If we've not already populated our map..
         */
        if ( table.size() < 1 )
        {
            DEBUG_LOG( "MIME-type table empty.  Populating" );

            /**
             * Attempt to parse /etc/mime.types.
             */
            if ( CFile::exists("/etc/mime.types" ) )
            {
                DEBUG_LOG( "/etc/mime.types is present.  Using it." );

                std::vector<std::string> data;
                pcrecpp::RE regex("(.+?)(?:[\\r\\n\\t]+|$)");

                std::ifstream file("/etc/mime.types", std::ios::in);
                if (file.is_open())
                {
                    std::string line;
                    std::string piece;

                    while (!file.eof())
                    {
                        getline(file, line);
                        pcrecpp::StringPiece input(line.c_str());
                        while (regex.Consume(&input, &piece))
                            data.push_back(piece);
                        for (size_t i = 1; i < data.size(); i++)
                            table[data[i]] = data[0];
                        data.clear();
                    }
                    file.close();
                }
            }

            /**
             * The previous code should have opened /etc/mime.types
             * and parsed it.
             *
             * However that file might be missing, or the parsing might
             * have failed.
             *
             * If that is the case then the map will be empty, so we'll
             * add some simple entries and leave it at that.
             */
            if ( table.size() < 1 )
            {
                DEBUG_LOG( "MIME-table empty.  Using our fixed list." );

                table[ "conf" ] = "text/plain";
                table[ "gif" ]  = "image/gif";
                table[ "htm" ]  = "text/html";
                table[ "html" ] = "text/html";
                table[ "jpeg" ] = "image/jpg";
                table[ "jpg" ]  = "image/jpg";
                table[ "png" ]  = "image/png";
                table[ "text" ] = "text/plain";
                table[ "txt" ]  = "text/plain";
            }
        }

        /**
         * Lookup the value, and use the default if we can't find a match.
         */
        std::string value = table[ extension ];

        DEBUG_LOG( "MIME type of " + filename + " is " + value );

        if ( value.empty() )
            value = default_type;

        lua_pushstring(L, value.c_str());
        return(1);
    }
    else
    {
        /**
         * No extension found.
         */
        lua_pushstring(L, default_type );
        return(1);
    }
}


/**
 * Attach a file via a mime-entity.
 */
bool mimetic_attach_file(lua_State *L, mimetic::MimeEntity *m, char* filename)
{
    std::filebuf ifile;
    std::ostringstream encoded;

    /**
     * Get the MIME-type of the file.
     */
    lua_pushstring(L, filename );
    if ( mime_type( L ) != 1 )
        return false;

    const char *type = lua_tostring(L,-1);

    ifile.open(filename, std::ios::in);
    if (ifile.is_open())
    {
        std::istream is(&ifile);
        mimetic::Attachment *at = new mimetic::Attachment(filename, mimetic::ContentType(type));
        mimetic::Base64::Encoder b64;
        std::ostreambuf_iterator<char> oi(encoded);
        std::istreambuf_iterator<char> ibegin(is), iend;
        encode(ibegin, iend, b64, oi);
        at->body().assign(encoded.str());
        m->body().parts().push_back(at);
        ifile.close();
        return true;
    }
    else {
        return false;
    }
    return false;
}

/**
 * Handle adding attachments to a plain mail.
 *
 * Given a file containing an email to be sent we must parse it sufficiently
 * well to build a MIME-based message instead.
 *
 * Then add any referenced files to it.
 *
 * This is horrid..
 *
 */
bool handle_attachments( lua_State *L, char *filename, std::vector<std::string> files )
{
    mimetic::MimeEntity *message = 0;
    mimetic::MimeVersion v1("1.0");

    /**
     * Open the file, and parse it into header + body.
     */
    bool in_header = true;

    std::ifstream input (filename);
    if ( !input.is_open() )
        return false;

    /**
     * Headers + body text.
     */
    std::vector<std::string> headers;
    std::string text;

    /**
     * Read line by line.
     */
    while( input.good() )
    {
        std::string line;
        getline( input, line );

        /**
         * If we're in the header store it away.
         */
        if ( in_header )
        {
            if ( line.length() <= 0 )
                in_header = false;
            else
                headers.push_back( line );
        }
        else
        {
            text += line;
            text += "\n";
        }
    }
    input.close();


    try {

        message = new  mimetic::MimeEntity;

        message->body().assign(text);
        message->header().contentType("text/plain; charset=utf-8");
        message->header().contentTransferEncoding("8bit");
        message->header().mimeVersion(v1);

        /**
         * Add files.
         */
        std::vector<std::string>::iterator it;
        for (it = files.begin(); it != files.end(); ++it)
        {
            std::string path = (*it);

            mimetic::MimeEntity *m = message;
            message = new mimetic::MultipartMixed();
            message->header().mimeVersion(v1);
            message->body().parts().push_back(m);
            mimetic_attach_file(L, message, (char *)path.c_str() );
        }

        /**
         * OK iterate over the headers we've received.
         */
        for (it = headers.begin(); it != headers.end(); ++it)
        {
            std::string head = (*it);

            std::size_t offset = head.find(":");
            if ( offset != std::string::npos )
            {
                std::string header = head.substr(0, offset);
                std::string value  = head.substr(offset+1);

                message->header().field( header ).value( value );
            }

        }

        /**
         * Get the message in a way we can examine.
         */
        std::ostringstream output;
        output << *message << std::endl;
        delete message;

        /**
         * Now overwrite our temporary file with the updated
         * version.
         */
        std::string content = output.str();
        std::ofstream myfile;
        myfile.open (filename);
        myfile << output.str();
        myfile.close();
        return( true );

    }
    catch(std::exception &e) {
        if (message)
            delete message;
        return false;
    }
    catch(std::string &e) {
        if (message)
            delete message;
        return false;
    }
    catch (...) {
        if (message)
            delete message;
        return false;
    }
}



/**
 * Count messages in the selected folder(s).
 */
int count_messages(lua_State * L)
{
    CGlobal *global = CGlobal::Instance();
    std::vector<CMessage *> *messages = global->get_messages();

    lua_pushinteger(L, messages->size() );
    return 1;
}


/**
 * Compose a new mail.
 */
int compose(lua_State * L)
{
    /**
     * Prompt for the recipient
     */
    lua_pushstring(L, "To: " );
    int ret = prompt( L);
    if ( ret != 1 )
    {
        lua_pushstring(L, "Error recieving recipient" );
        return( msg(L ) );

    }
    const char *recipient = lua_tostring(L,-1);

    /**
     * Prompt for subject.
     */
    lua_pushstring(L, "Subject: " );
    ret = prompt( L);
    if ( ret != 1 )
    {
        lua_pushstring(L, "Error recieving subject" );
        return( msg(L ) );
    }

    const char *subject = lua_tostring(L,-1);

    CGlobal *global = CGlobal::Instance();
    std::string *from   = global->get_variable( "from" );

    /**
     * Generate a temporary file for the message body.
     */
    char filename[] = "/tmp/mytemp.XXXXXX";
    int fd = mkstemp(filename);

    if (fd == -1)
        return luaL_error(L, "Failed to create a temporary file.");

    /**
     * .signature handling.
     */
    std::string sig = "";

    lua_getglobal(L, "get_signature" );
    if (lua_isfunction(L, -1))
    {
        lua_pushstring(L, from->c_str() );
        lua_pushstring(L, recipient );
        lua_pushstring(L, subject );
	if (! lua_pcall(L, 3, 1, 0) )
        {
            sig = lua_tostring(L,-1);
        }
    }

    /**
     * To
     */
    unused=write(fd, "To: ", strlen( "To: "));
    unused=write(fd, recipient, strlen( recipient ));
    unused=write(fd, "\n", 1 );

    /**
     * Subject.
     */
    unused=write(fd, "Subject: ", strlen( "Subject: " ) );
    unused=write(fd, subject, strlen( subject ) );
    unused=write(fd, "\n", 1 );

    /**
     * From
     */
    unused=write(fd, "From: " , strlen( "From: " ) );
    unused=write(fd, from->c_str(), strlen( from->c_str() ) );
    unused=write(fd, "\n", 1 );

    /**
     * Space
     */
    unused=write(fd, "\n", 1 );

    /**
     * Body:  User will fill out.
     */

    /**
     * .sig
     */
    if ( sig.empty() )
    {
        unused=write(fd, "\n-- \n", strlen("\n-- \n" ) );
    }
    else
    {
        unused=write(fd, sig.c_str(), sig.size() );
    }

    close(fd);

    /**
     * Save the current state of the TTY
     */
    refresh();
    def_prog_mode();
    endwin();

    /**
     * Get the editor.
     */
    std::string cmd = get_editor();

    /**
     * Run the editor.
     */
    cmd += " ";
    cmd += filename;
    unused = system(cmd.c_str());


    /**
     * Attachments associated with this mail.
     */
    std::vector<std::string> attachments;


    /**
     * Prompt for confirmation.
     */
    bool cont = true;


    while( cont )
    {
        /**
         * Use prompt_chars() to get the input
         */
        lua_pushstring(L,"Send mail?  y/n/a>" );
        lua_pushstring(L,"anyANY");

        ret = prompt_chars(L);
        if ( ret != 1 )
        {
            lua_pushstring(L, "Error recieving confiramtion." );
            return( msg(L ) );
        }
        const char * response = lua_tostring(L, -1);


        if (  ( response[0] == 'y' ) ||
              ( response[0] == 'Y' ) )
        {
            cont = false;
        }
        if ( ( response[0] == 'n' ) ||
             ( response[0] == 'N' ) )
        {
            cont = false;
            unlink( filename );
            reset_prog_mode();
            refresh();

            lua_pushstring(L, SENDING_ABORTED);
            return( msg(L ) );
        }
        if (  ( response[0] == 'a' ) ||
              ( response[0] == 'A' ) )
        {
            /**
             * Add attachment.
             */
            lua_pushstring(L,"Path to attachment?" );
            ret = prompt( L);
            if ( ret != 1 )
            {
                unlink( filename );
                reset_prog_mode();
                refresh();
                lua_pushstring(L, "Error receiving attachment." );
                return( msg(L ) );
            }
            const char * path = lua_tostring(L, -1);
            if ( path != NULL )
            {
                attachments.push_back( path );
            }
        }
    }


    /**
     * If attachments are non-empty we need to hadnle them.
     */
    if ( attachments.size() > 0 )
    {
        handle_attachments( L, filename, attachments );
    }


    /**
     * OK now we're going to send the mail.  Get some settings.
     */
    std::string *sendmail  = global->get_variable("sendmail_path");
    std::string *sent_path = global->get_variable("sent_mail");

    /**
     * Send the mail.
     */
    CFile::file_to_pipe( filename, *sendmail );

    /**
     * Get a filename in the sent-mail path.
     */
    std::string archive = CMaildir::message_in( *sent_path, false );
    if ( archive.empty() )
    {
        unlink( filename );
        reset_prog_mode();
        refresh();

        lua_pushstring(L, "error finding save path");
        return( msg(L ) );
    }


    /**
     * If we got a filename then copy the mail there.
     */
    CFile::copy( filename, archive );

    unlink( filename );

    /**
     * Reset + redraw
     */
    reset_prog_mode();
    refresh();

    return 0;
}


/**
 * Reply to an existing mail.
 */
int reply(lua_State * L)
{
    /**
     * Get the message we're replying to.
     */
    CMessage *mssg = get_message_for_operation( NULL );
    if ( mssg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }


    /**
     * Get the subject, and sender, etc.
     */
    std::string subject = mssg->subject();
    std::string to      = mssg->from();
    std::string ref     = mssg->header( "Message-ID" );


    CGlobal *global     = CGlobal::Instance();
    std::string *from   = global->get_variable( "from" );

    /**
     * Generate a temporary file for the message body.
     */
    char filename[] = "/tmp/lumail.reply.XXXXXX";
    int fd = mkstemp(filename);

    if (fd == -1)
    {
        return luaL_error(L, "Failed to create a temporary file.");
    }

    /**
     * To
     */
    unused=write(fd, "To: ", strlen( "To: "));
    unused=write(fd, to.c_str(), strlen( to.c_str() ));
    unused=write(fd, "\n", 1 );

    /**
     * Subject.
     */
    unused=write(fd, "Subject: ", strlen( "Subject: " ) );
    unused=write(fd, subject.c_str(), strlen( subject.c_str() ) );
    unused=write(fd, "\n", 1 );

    /**
     * From
     */
    unused=write(fd, "From: " , strlen( "From: " ) );
    unused=write(fd, from->c_str(), strlen( from->c_str() ) );
    unused=write(fd, "\n", 1 );

    /**
     * If we have a message-id add that to the references.
     */
    if ( !ref.empty() )
    {
        /**
         * Message-ID might look like this:
         *
         * Message-ID: <"str"> ("comment")
         *
         * Remove the comment.
         */
        unsigned int start = 0;
        if ( ( ref.find('(') ) != std::string::npos )
        {
            unsigned int end = ref.find(')',start);
            if ( end != std::string::npos )
                ref.erase(start,end-start+1);
        }

        /**
         * If still non-empty ..
         */
        if ( !ref.empty() )
        {
            unused=write(fd, "References: " , strlen( "References: " ) );
            unused=write(fd, ref.c_str(), strlen( ref.c_str() ) );
            unused=write(fd, "\n", 1 );
        }
    }

    /**
     * Space
     */
    unused=write(fd, "\n", 1 );

    /**
     * Body
     */
    std::vector<std::string> body = mssg->body();
    int lines =(int)body.size();
    for( int i = 0; i < lines; i++ )
    {
        unused=write(fd, "> ", 2 );
        unused=write(fd, body[i].c_str(), strlen(body[i].c_str() ));
        unused=write(fd, "\n", 1 );
    }

    unused=write(fd, "-- \n", strlen("-- \n" ) );
    close(fd);

    /**
     * Save the current state of the TTY
     */
    refresh();
    def_prog_mode();
    endwin();

    /**
     * Get the editor.
     */
    std::string cmd = get_editor();

    /**
     * Run the editor.
     */
    cmd += " ";
    cmd += filename;
    unused = system(cmd.c_str());


    /**
     * Attachments associated with this mail.
     */
    std::vector<std::string> attachments;


    /**
     * Prompt for confirmation.
     */
    bool cont = true;
    int ret;


    while( cont )
    {
        /**
         * Use prompt_chars() to get the input
         */
        lua_pushstring(L,"Send mail?  y/n/a>" );
        lua_pushstring(L,"anyANY");

        ret = prompt_chars(L);
        if ( ret != 1 )
        {
            lua_pushstring(L, "Error recieving confiramtion." );
            return( msg(L ) );
        }

        const char * response = lua_tostring(L, -1);


        if (  ( response[0] == 'y' ) ||
              ( response[0] == 'Y' ) )
        {
            cont = false;
        }
        if ( ( response[0] == 'n' ) ||
             ( response[0] == 'N' ) )
        {
            cont = false;
            unlink( filename );
            reset_prog_mode();
            refresh();

            lua_pushstring(L, SENDING_ABORTED);
            return( msg(L ) );
        }
        if ( ( response[0] == 'a' ) ||
             ( response[0] == 'A' ) )
        {
            /**
             * Add attachment.
             */
            lua_pushstring(L,"Path to attachment?" );
            ret = prompt( L);
            if ( ret != 1 )
            {
                unlink( filename );
                reset_prog_mode();
                refresh();
                lua_pushstring(L, "Error receiving attachment." );
                return( msg(L ) );
            }
            const char * path = lua_tostring(L, -1);
            if ( path != NULL )
            {
                attachments.push_back( path );
            }
        }
    }

    /**
     * If attachments are non-empty we need to hadnle them.
     */
    if ( attachments.size() > 0 )
    {
        handle_attachments( L, filename, attachments );
    }

    /**
     * OK now we're going to send the mail.  Get some settings.
     */
    std::string *sent_path = global->get_variable("sent_mail");
    std::string *sendmail  = global->get_variable("sendmail_path");


    /**
     * Send the mail.
     */
    CFile::file_to_pipe( filename, *sendmail );

    /**
     * Get a filename in the sent-mail path.
     */
    std::string archive = CMaildir::message_in( *sent_path, false );
    if ( archive.empty() )
    {
        unlink( filename );
        reset_prog_mode();
        refresh();

        lua_pushstring(L, "error finding save path");
        return( msg(L ) );
    }


    /**
     * If we got a filename then copy the mail there.
     */
    CFile::copy( filename, archive );

    unlink( filename );

    /**
     * Now we're all cleaned up mark the orignal message
     * as being replied to.
     */
    mssg->add_flag( 'R' );


    /**
     * Reset + redraw
     */
    reset_prog_mode();
    refresh();

    return( 0 );
}


/**
 * Send an email via lua-script.
 */
int send_email(lua_State *L)
{
    /**
     * Get the values.
     */
    lua_pushstring(L, "to" );
    lua_gettable(L,-2);
    const char *to = lua_tostring(L, -1);
    lua_pop(L, 1);
    if (to == NULL) {
	return luaL_error(L, "Missing recipient.");
    }

    lua_pushstring(L, "from" );
    lua_gettable(L,-2);
    const char *from = lua_tostring(L, -1);
    lua_pop(L, 1);
    if (from == NULL) {
	return luaL_error(L, "Missing sender.");
    }

    lua_pushstring(L, "subject" );
    lua_gettable(L,-2);
    const char *subject = lua_tostring(L, -1);
    lua_pop(L, 1);
    if (subject == NULL) {
	return luaL_error(L, "Missing subject.");
    }

    lua_pushstring(L, "body" );
    lua_gettable(L,-2);
    const char *body = lua_tostring(L, -1);
    lua_pop(L, 1);
    if (body == NULL) {
	return luaL_error(L, "Missing body.");
    }

    /**
     * Optional attachments.
     */
    lua_pushstring(L,"attachments" );
    lua_gettable(L,-2);
    std::vector<std::string> filenames;
    if ( lua_istable(L, -1 ) )
    {
        lua_pushnil(L);

        while (lua_next(L, -2))
        {
            const char *path  = lua_tostring(L, -1);
            filenames.push_back( path );
            lua_pop( L , 1);
        }

        lua_pop(L , 1); // pop nil
    }
    else
        lua_pop(L, 1);


    /**
     * Now send the mail.
     */
    char filename[] = "/tmp/mytemp.XXXXXX";
    int fd = mkstemp(filename);
    if (fd == -1)
        return luaL_error(L, "Failed to create a temporary file.");

    /**
     * .signature handling.
     */
    std::string sig = "";

    lua_getglobal(L, "get_signature" );
    if (lua_isfunction(L, -1))
    {
        lua_pushstring(L, from );
        lua_pushstring(L, to );
        lua_pushstring(L, subject );
	if (! lua_pcall(L, 3, 1, 0) )
        {
            sig = lua_tostring(L,-1);
        }
    }

    /**
     * To
     */
    unused=write(fd, "To: ", strlen( "To: "));
    unused=write(fd, to, strlen( to ));
    unused=write(fd, "\n", 1 );

    /**
     * Subject.
     */
    unused=write(fd, "Subject: ", strlen( "Subject: " ) );
    unused=write(fd, subject, strlen( subject ) );
    unused=write(fd, "\n", 1 );

    /**
     * From
     */
    unused=write(fd, "From: " , strlen( "From: " ) );
    unused=write(fd, from, strlen( from ) );
    unused=write(fd, "\n", 1 );

    /**
     * Space
     */
    unused=write(fd, "\n", 1 );

    /**
     * Body.
     */
    unused=write(fd, body, strlen( body ) );

    /**
     * .sig
     */
    if ( sig.empty() )
    {
        unused=write(fd, "\n\n-- \n", strlen("\n\n-- \n" ) );
    }
    else
    {
        unused=write(fd, "\n", 1 );
        unused=write(fd, "\n", 1 );
        unused=write(fd, sig.c_str(), sig.size() );
    }

    close(fd);


    /**
     * OK now we're going to send the mail.  Get some settings.
     */
    CGlobal *global        = CGlobal::Instance();
    std::string *sendmail  = global->get_variable("sendmail_path");
    std::string *sent_path = global->get_variable("sent_mail");

    if ( filenames.size() > 0 )
    {
        handle_attachments( L, filename, filenames );
    }


    /**
     * Send the mail.
     */
    CFile::file_to_pipe( filename, *sendmail );

    /**
     * Get a filename in the sentmail path.
     */
    std::string archive = CMaildir::message_in( *sent_path, true );
    if ( archive.empty() )
    {
        unlink( filename );
        reset_prog_mode();
        refresh();

        lua_pushstring(L, "error finding save path");
        return( msg(L ) );
    }


    /**
     * If we got a filename then copy the mail there.
     */
    CFile::copy( filename, archive );

    unlink( filename );


    return 0;
}


/**
 * Get the screen width.
 */
int screen_width(lua_State * L)
{
    lua_pushinteger(L, CScreen::width() );
    return 1;
}


/**
 * Get the screen height.
 */
int screen_height(lua_State * L)
{
    lua_pushinteger(L, CScreen::height() );
    return 1;
}

/**
 * Return a table of all known variables and their current
 * settings.
 */
int get_variables(lua_State *L )
{
    CGlobal *global = CGlobal::Instance();
    std::unordered_map<std::string,std::string *>  vars = global->get_variables();
    std::unordered_map<std::string, std::string *>::iterator iter;

    /**
     * Create the table.
     */
    lua_newtable(L);

    int i = 1;
    for (iter = vars.begin(); iter != vars.end(); ++iter )
    {
        std::string name = iter->first;
        std::string *val = iter->second;

        lua_pushstring(L,name.c_str() );

        if ( val != NULL )
            lua_pushstring(L,val->c_str());
        else
            lua_pushstring(L, "NULL" );

        lua_settable(L,-3);
        i++;
    }

    return 1;
}



/**
 * File/Utility handlers. Useful for writing portable configuration files.
 */
int file_exists(lua_State *L)
{
    const char *str = lua_tostring(L, -1);
    if (str == NULL)
	return luaL_error(L, "Missing argument to file_exists(..)");

    if ( CFile::exists( str ) )
        lua_pushboolean(L,1);
    else
        lua_pushboolean(L,0);

    return 1;
}

/**
 * Is the given path a directory.
 */
int is_directory(lua_State *L)
{
    const char *str = lua_tostring(L, -1);
    if (str == NULL)
	return luaL_error(L, "Missing argument to is_directory(..)");

    if ( CFile::is_directory( str ) )
        lua_pushboolean(L,1);
    else
        lua_pushboolean(L,0);

    return 1;
}

/**
 * Is the given path an executable?
 */
int executable(lua_State *L)
{
    const char *str = lua_tostring(L, -1);
    if (str == NULL)
	return luaL_error(L, "Missing argument to is_directory(..)");

    if ( CFile::executable( str ) )
        lua_pushboolean(L,1);
    else
        lua_pushboolean(L,0);

    return 1;
}

/**
 * Attachment handling.
 */
int attachments(lua_State *L)
{
    /**
     * Get the path (optional).
     */
    const char *str = lua_tostring(L, -1);

    CMessage *msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }
    else
    {
        /**
         * Count the attachments.
         */
        std::vector<std::string> attachments = msg->attachments();
        std::vector<std::string>::iterator it;


        /**
         * create a new table.
         */
        lua_newtable(L);

        /**
         * Lua indexes start at one.
         */
        int i = 1;


        /**
         * For each attachment, add it to the table.
         */
        for (it = attachments.begin(); it != attachments.end(); ++it)
        {
            std::string name = (*it);


            lua_pushnumber(L,i);
            lua_pushstring(L,name.c_str());
            lua_settable(L,-3);
            i++;
        }
    }

    if ( str != NULL )
        delete( msg );

    return( 1 );

}


/**
 * Attachment handling.
 */
int count_attachments(lua_State *L)
{
    /**
     * Get the path (optional).
     */
    const char *str = lua_tostring(L, -1);
    int ret = 0;

    CMessage *msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }
    else
    {
        /**
         * Count the attachments.
         */
        std::vector<std::string> attachments = msg->attachments();
        int count = attachments.size();

        /**
         * Setup the return values.
         */
        lua_pushinteger(L, count );
        ret = 1;
    }

    if ( str != NULL )
        delete( msg );

    return( ret );

}



/**
 * Attachment handling.
 */
int save_attachment(lua_State *L)
{
    /**
     * Get the path to save to.
     */
    int offset       = lua_tointeger(L,-2);
    const char *path = lua_tostring(L, -1);
    bool ret;

    CMessage *msg = get_message_for_operation(NULL);
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }
    else
    {
        /**
         * Count the attachments.
         */
        std::vector<std::string> attachments = msg->attachments();
        int count                            = attachments.size();

        /**
         * Out of range: return false.
         */
        if ( ( offset < 1 ) || ( offset > count ) )
        {
            lua_pushboolean(L,0);
            return 1;
        }

        /**
         * Save the message.
         */
        ret = msg->save_attachment( offset, path );
    }


    if ( ret )
        lua_pushboolean(L, 1 );
    else
        lua_pushboolean(L, 0 );
    return( 1 );

}


/**
 * Dump the Lua stack to our debug-log.
 *
 * NOTE: Only present if lumail is compiled with debugging.
 */
int lua_dump_stack(lua_State *L)
{
#ifdef LUMAIL_DEBUG
    CLua *lua = CLua::Instance();
    lua->dump_stack();
#endif
    return 0;
}

/**
 * Log a message to the debug-log.
 *
 * NOTE: Only present if lumail is compiled with debugging.
 */
int log_message(lua_State *L)
{
#ifdef LUMAIL_DEBUG
    const char *str = lua_tostring(L, -1);
    if (str == NULL)
	return luaL_error(L, "Missing argument to log_message(..)");

    DEBUG_LOG( str );
#endif
    return 0;
}

/**
 * Load *.lua from the given directory.
 */
int load_directory(lua_State *L)
{

    const char *path = lua_tostring(L, -1);
    if (path == NULL)
	return luaL_error(L, "Missing argument to load_directory(..)");

    /**
     * The lua intepretter.
     */
    CLua *lua = CLua::Instance();

    int count = 0;
    dirent *de;
    DIR *dp;

    dp = opendir(path);
    if (dp)
    {
        std::string base = path;
        base += "/";

        while (true)
        {
            de = readdir(dp);
            if (de == NULL)
                break;

            /**
             * Build up the complete file.
             */
            std::string file = base + de->d_name;

            /**
             * If the file ends in .lua then load it.
             */
            size_t offset = file.rfind('.');
            if(offset != std::string::npos)
            {
                /**
                 * Get the lower-case version.
                 */
                std::string extension = file.substr(offset+1);
                std::transform(extension.begin(), extension.end(), extension.begin(), tolower );

                if ( strcmp( "lua", extension.c_str() ) == 0 )
                {
                    lua->load_file( file );
                    count += 1;
                }

            }
        }
        closedir(dp);
    }

    /**
     * Return the number of files read.
     */
    lua_pushinteger(L, count );
    return 1;
}
