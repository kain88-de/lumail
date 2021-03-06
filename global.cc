/**
 * global.cc - Singleton interface to store global data
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

#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string.h>
#include <stdlib.h>

#include "debug.h"
#include "global.h"

/**
 * Instance-handle.
 */
CGlobal *CGlobal::pinstance = NULL;

/**
 * Get access to our singleton-object.
 */
CGlobal *CGlobal::Instance()
{
    if (!pinstance)
	pinstance = new CGlobal;

    return pinstance;
}

/**
 * Constructor - This is private as this class is a singleton.
 */
CGlobal::CGlobal()
{
    /**
     * Defaults.
     */
    m_cur_folder     = 0;
    m_cur_message    = 0;
    m_msg_offset     = 0;
    m_messages       = NULL;

    /**
     * Defaults as set in our variable hash-map.
     */
    set_variable( "editor", new std::string("/usr/bin/vim") );
    set_variable( "global_mode",   new std::string("maildir"));
    set_variable( "index_format",  new std::string( "[$FLAGS] $FROM - $SUBJECT" ) );
    set_variable( "index_limit",   new std::string("all") );
    set_variable( "maildir_format", new std::string( "$CHECK - $PATH" ) );
    set_variable( "message_filter", new std::string("") );
    set_variable( "maildir_limit", new std::string("all") );
    set_variable( "sendmail_path", new std::string( "/usr/lib/sendmail -t" ) );


    /**
     * Default colours.
     */
    set_variable( "attachment_colour",     new std::string( "white" ) );
    set_variable( "body_colour",           new std::string( "white" ) );
    set_variable( "header_colour",         new std::string( "white" ) );
    set_variable( "unread_maildir_colour", new std::string( "red" ) );
    set_variable( "unread_message_colour", new std::string( "red" ) );

    /**
     * From address is a little fiddly.
     */
    std::string user = "UNKNOWN";
    if ( getenv( "USER" ) )
        user = getenv( "USER" );
    std::string *from = new std::string( user + "@localhost" );
    set_variable( "from", from );
}

/**
 * Get all selected folders.
 */
std::vector<std::string> CGlobal::get_selected_folders()
{
    return (m_selected_folders);
}

/**
 * Get all folders.
 */
std::vector<CMaildir> CGlobal::get_all_folders()
{
    std::vector<CMaildir> maildirs;

    /**
     * Maildir prefix.
     */
    CGlobal *global     = CGlobal::Instance();
    std::string *prefix = global->get_variable( "maildir_prefix" );

    std::vector<std::string> folders =
	CMaildir::getFolders(*prefix);
    std::vector < std::string >::iterator it;
    for (it = folders.begin(); it != folders.end(); ++it) {
	maildirs.push_back(CMaildir(*it));
    }

    return (maildirs);
}

/**
 * Get folders matching the current mode.
 */
std::vector < CMaildir > CGlobal::get_folders()
{
    CGlobal *global = CGlobal::Instance();
    std::vector<CMaildir> folders = global->get_all_folders();
    std::vector<CMaildir> display;
    std::string * filter = global->get_variable("maildir_limit");

    /**
     * Filter the folders to those we can display
     */
    std::vector<CMaildir>::iterator it;
    for (it = folders.begin(); it != folders.end(); ++it) {
        CMaildir x = *it;

        if ( x.matches_filter( filter ) )
            display.push_back(x);
    }

    return (display);
}

/**
 * My sort function: sort CMessages by most recent to oldest.
 */
bool my_sort(CMessage *a, CMessage *b)
{
    /**
     * Stat both files.
     */
    struct stat us;
    struct stat them;

    std::string us_path = a->path();
    std::string them_path = b->path();

    if (stat(us_path.c_str(), &us) < 0)
	return 0;

    if (stat(them_path.c_str(), &them) < 0)
	return 0;

    return (us.st_mtime < them.st_mtime);

}

/**
 * Get all messages from the currently selected folders.
 */
std::vector<CMessage *>* CGlobal::get_messages()
{
    return( m_messages );
}


/**
 * Update the list of global messages, using the index_limit string set by lua.
 */
void CGlobal::update_messages()
{
    /**
     * If we have items already then free each of them.
     */
    if ( m_messages != NULL ) {
        std::vector<CMessage *>::iterator it;
        for (it = m_messages->begin(); it != m_messages->end(); ++it) {
            delete( *it );
        }
        delete( m_messages );
    }

    /**
     * create a new store.
     */
    m_messages = new std::vector<CMessage *>;


    /**
     * Get the selected maildirs.
     */
    CGlobal *global = CGlobal::Instance();
    std::vector<std::string> folders = global->get_selected_folders();
    std::string * filter = global->get_variable("index_limit" );


    /**
     * For each selected maildir read the messages.
     */
    std::vector<std::string>::iterator it;
    for (it = folders.begin(); it != folders.end(); ++it) {

        /**
         * get the messages from this folder.
         */
        CMaildir tmp = CMaildir(*it);
        std::vector<CMessage *> contents = tmp.getMessages();

        /**
         * Append to the list of messages combined.
         */
        std::vector<CMessage *>::iterator mit;
        for (mit = contents.begin(); mit != contents.end(); ++mit) {
            if ( (*mit)->matches_filter( filter ) )
                m_messages->push_back(*mit) ;
        }
    }

    /*
     * Sort?
     */
    std::sort(m_messages->begin(), m_messages->end(), my_sort);

}

/**
 * Remove all selected folders.
 */
void CGlobal::unset_folders()
{
    m_selected_folders.clear();
}

/**
 * Add a folder to the selected set.
 */
void CGlobal::add_folder(std::string path)
{
    m_selected_folders.push_back(path);
}

/**
 * Remove a folder from the selected set.
 */
bool CGlobal::remove_folder(std::string path)
{
    std::vector<std::string>::iterator it;

    /**
     * Find the folder.
     */
    it = std::find(m_selected_folders.begin(), m_selected_folders.end(), path);

    /**
     * If we found it reemove it.
     */
    if (it != m_selected_folders.end()) {
	m_selected_folders.erase(it);
	return true;
    }

    /**
     * Failed to find it.
     */
    return false;

}


/**
 * Get the value of the named string-variable.
 */
std::string * CGlobal::get_variable( std::string name )
{

    std::string *value = m_variables[name];

#ifdef LUMAIL_DEBUG

    std::string dm = "Get variable named '";
    dm += name ;
    dm += "' value is " ;

    if ( value != NULL )
        dm += *value;
    else
        dm += " NULL";

    DEBUG_LOG( dm );

#endif /* LUMAIL_DEBUG */

    /**
     * Get the value.
     */
    return( value );
}


/**
 * Update the value of the named string variable.
 */
void CGlobal::set_variable( std::string name, std::string *value )
{
    /**
     * Free the current value, if one is set.
     */
    std::string *current = m_variables[name];
    if ( current != NULL )
        delete( current );

    /**
     * Store new value.
     */
    m_variables[ name ] = value;


    std::string dm = "Set variable named '" ;
    dm += name ;
    dm += "' to value '";
    dm += *value;
    dm += "'";

    DEBUG_LOG( dm );

}


/**
 * Return our map of variables to the caller.
 */
std::unordered_map<std::string, std::string *> CGlobal::get_variables()
{
    return( m_variables );
}
