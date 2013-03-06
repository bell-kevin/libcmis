/* libcmis
 * Version: MPL 1.1 / GPLv2+ / LGPLv2+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2013 Cao Cuong Ngo <cao.cuong.ngo@gmail.com>
 *
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPLv2+"), or
 * the GNU Lesser General Public License Version 2 or later (the "LGPLv2+"),
 * in which case the provisions of the GPLv2+ or the LGPLv2+ are applicable
 * instead of those above.
 */

#include "session-factory.hxx"
#include "oauth2-handler.hxx"
#include "json-utils.hxx"
#include "xml-utils.hxx"

using namespace std;

OAuth2Handler::OAuth2Handler( BaseSession* session, libcmis::OAuth2DataPtr data )
    throw ( libcmis::Exception ) :
        m_session( session ),
        m_data( data ),
        m_access( ),
        m_refresh( )
{
    if ( !m_data )
        m_data.reset( new libcmis::OAuth2Data() );
}

OAuth2Handler::OAuth2Handler( const OAuth2Handler& copy ) :
        m_session( copy.m_session ),
        m_data( copy.m_data ),
        m_access( copy.m_access ),
        m_refresh( copy.m_refresh )
{
}

OAuth2Handler::OAuth2Handler( ):
        m_session( NULL ),
        m_data( ),
        m_access( ),
        m_refresh( )
{
    m_data.reset( new libcmis::OAuth2Data() );
}

OAuth2Handler& OAuth2Handler::operator=( const OAuth2Handler& copy )
{
    if ( this != &copy )
    {
        m_session = copy.m_session;
        m_data = copy.m_data;
        m_access = copy.m_access;
        m_refresh = copy.m_refresh;
    }

    return *this;
}

OAuth2Handler::~OAuth2Handler( )
{
}

char* OAuth2Handler::authenticate( const string& username, const string& password )
{
    char* authCode = NULL;
    cout << "HERE";
    if ( m_data->getAuthCodeProvider() != NULL )
    {
        string url = getAuthURL();
        authCode = m_data->getAuthCodeProvider()( url.c_str(), username.c_str(), password.c_str() );
    }
    cout << "HERE2" << authCode;
    return authCode;
}

void OAuth2Handler::fetchTokens( string authCode ) throw ( libcmis::Exception )
{
    string post =
        "code="              + authCode +
        "&client_id="        + m_data->getClientId() +
        "&client_secret="    + m_data->getClientSecret() +
        "&redirect_uri="     + m_data->getRedirectUri() +
        "&grant_type=authorization_code" ;

    istringstream is( post );

    libcmis::HttpResponsePtr resp = m_session->httpPostRequest ( m_data->getTokenUrl(), is,
            "application/x-www-form-urlencoded" );

    Json jresp = Json::parse( resp->getStream()->str() );
    m_access = jresp["access_token"].toString();
    m_refresh = jresp["refresh_token"].toString();
}

void OAuth2Handler::refresh( ) throw ( libcmis::Exception )
{
    string post =
        "refresh_token="     + m_refresh +
        "&client_id="        + m_data->getClientId() +
        "&client_secret="    + m_data->getClientSecret() +
        "&grant_type=refresh_token" ;

    istringstream is( post );

    libcmis::HttpResponsePtr resp = m_session->httpPostRequest ( m_data->getTokenUrl(), is,
            "application/x-www-form-urlencoded" );

    Json jresp = Json::parse( resp->getStream()->str() );
    m_access = jresp["access_token"].toString();
}

string OAuth2Handler::getAuthURL( )
{
    return m_data->getAuthUrl() + "?scope=" +
            libcmis::escape( m_data->getScope() ) + "+" +
            "&redirect_uri="+ m_data->getRedirectUri() +
            "&response_type=code"
            "&client_id="+ m_data->getClientId();
}

string OAuth2Handler::getRefreshToken( ) throw ( libcmis::Exception )
{
    return m_refresh ;
}

string OAuth2Handler::getAccessToken( ) throw ( libcmis::Exception )
{
    return m_access ;
}

string OAuth2Handler::getHttpHeader( ) throw ( libcmis::Exception )
{
    return "Authorization: Bearer " + m_access ;
}