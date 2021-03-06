/**
 * This is code for dumping a Maildir messages.
 *
 * It is designed to be helpful when debugging MIME-related issues.
 *
 * Steve
 * --
 */

#include <stdint.h>
#include <iostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <mimetic/mimetic.h>



/**
 * Given a pointer to a message we want to walk all the MIME-parts
 * and return the *first* one that matches the given mime-type.
 *
 * (The given MIME-type will be text/plain.)
 */
std::string getMimePart(mimetic::MimeEntity* pMe, std::string mtype )
{
    mimetic::Header& h = pMe->header();

    /**
     * If the header matches then return.
     */
    std::string enc = h.contentTransferEncoding().mechanism();
    std::string type= h.contentType().str();

    if ( type.find( mtype ) != std::string::npos )
    {
        std::string body = (pMe)->body();
        std::string decoded;

        if (strcasecmp(enc.c_str(), "quoted-printable" ) == 0 )
        {
            mimetic::QP::Decoder qp;
            decode(body.begin(), body.end(), qp, std::back_inserter(decoded));
            return( decoded );
        }
        if (strcasecmp(enc.c_str(), "base64" ) == 0 )
        {
            mimetic::Base64::Decoder b64;
            decode(body.begin(), body.end(), b64, std::back_inserter(decoded));
            return( decoded );
        }

        return( body );
    }

    /**
     * Iterate over any sub-parts.
     */
    mimetic::MimeEntityList& parts = pMe->body().parts();
    mimetic::MimeEntityList::iterator mbit = parts.begin(), meit = parts.end();
    for(; mbit != meit; ++mbit)
    {

        /**
         * get the encoding and content-type.
         */
        mimetic::Header& mh = (*mbit)->header();
        std::string enc = mh.contentTransferEncoding().mechanism();
        std::string type= mh.contentType().str();

        /**
         * See if the part matches directly.
         */
        if ( type.find( mtype ) != std::string::npos )
        {
            std::string body = (*mbit)->body();
            std::string decoded;

            if (strcasecmp(enc.c_str(), "quoted-printable" ) == 0 )
            {
                mimetic::QP::Decoder qp;
                decode(body.begin(), body.end(), qp, std::back_inserter(decoded));
                return( decoded );
            }
            if (strcasecmp(enc.c_str(), "base64" ) == 0 )
            {
                mimetic::Base64::Decoder b64;
                decode(body.begin(), body.end(), b64, std::back_inserter(decoded));
                return( decoded );
            }

            return( body );
        }

        /**
         * There are also nested parts because MIME is nasty.
         */
        mimetic::MimeEntityList& np = (*mbit)->body().parts();
        mimetic::MimeEntityList::iterator bit = np.begin(), eit = np.end();
        for(; bit != eit; ++bit)
        {
            /**
             * See if a nested entry matches.
             */
            mimetic::Header& nh = (*bit)->header();

            /**
             * Get the encoding-type and the content-type.
             */
            std::string enc = nh.contentTransferEncoding().mechanism();
            std::string type= nh.contentType().str();

            /**
             * If the type matches, then return the (decoded?) body
             */
            if ( type.find( mtype ) != std::string::npos )
            {
                std::string body = (*bit)->body();
                std::string decoded;

                if (strcasecmp(enc.c_str(), "quoted-printable" ) == 0 )
                {
                    mimetic::QP::Decoder qp;
                    decode(body.begin(), body.end(), qp, std::back_inserter(decoded));
                    return( decoded );
                }
                if (strcasecmp(enc.c_str(), "base64" ) == 0 )
                {

                    mimetic::Base64::Decoder b64;
                    decode(body.begin(), body.end(), b64, std::back_inserter(decoded));
                    return( decoded );
                }

                return( body );
            }
        }
    }
    return "";
}


/**
 * Dump a single message.
 */
void dump_message( std::string filename )
{
    /**
     * Get the entity.
     */
    std::ifstream file(filename.c_str());
    mimetic::MimeEntity *me = new mimetic::MimeEntity(file);

    /**
     * Output the filename.
     */
    std::cout << filename << std::endl;
    std::cout << std::endl;

    /**
     * Show some headers.
     */
    mimetic::Header & h = me->header();
    std::cout << "From: " << (h.from()) << std::endl;
    std::cout << "Subject: " << (h.subject()) << std::endl;
    std::cout << "To: " << (h.to()) << std::endl;
    std::cout << std::endl;


    /**
     * Iterate over every MIME-part looking for a text/plain part.
     */
    std::string body = getMimePart( me, "text/plain" );
    bool found = ( !body.empty() );


    /**
     * If we didn't find a text/plain part then use the whole damn thing.
     */
    if ( ! found )
    {
        std::cout << "Didn't find MIME-part.  Hoping for the best" << std::endl;
        body = me->body();
    }


    /**
     * Split the body into an array, by newlines.
     */
    std::vector<std::string> result;
    std::stringstream stream(body);
    std::string line;
    while (std::getline(stream, line))
    {
        result.push_back( line );
    }


    /**
     * Now output the message.
     */
    std::vector<std::string>::iterator it;
    for (it = result.begin(); it != result.end(); ++it) {
        std::string x = *it;
        std::cout << x << std::endl;
    }


    /**
     * Cleanup.
     */
    delete(me);
    std::cout << "All done." << std::endl;
}




/**
 * For each named file, dump the message.
 */
int main( int argc, char *argv[] )
{
    if ( argc < 2 )
    {
        std::cout << "Usage " << argv[0] << " message-file" << std::endl;
        return 1;
    }

    for( int i = 1; i < argc; i++ )
    {
        dump_message( argv[i] );
    }

    exit( 0 );
}
