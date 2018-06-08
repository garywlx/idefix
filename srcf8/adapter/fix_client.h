/*!
 *  Copyright(c)2018, Arne Gockeln. All rights reserved.
 *  http://www.arnegockeln.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef IDEFIX_FIX_CLIENT_H
#define IDEFIX_FIX_CLIENT_H

#include <fix8/f8config.h>
#include <fix8/f8includes.hpp>
#include <fix8/session.hpp>
#include "fix_router.h"

class fix_client : public FIX8::Session
{
  fix_router _router;

public:
  fix_client(const FIX8::F8MetaCntx& ctx, const FIX8::SessionID& sid, FIX8::Persister *persist=0,
                         FIX8::Logger *logger=0, FIX8::Logger *plogger=0) : Session(ctx, sid, persist, logger, plogger), _router(*this) {}

  // Override these methods if required but remember to call the base class method first.
  bool handle_logon(const unsigned seqnum, const FIX8::Message *msg);

  // Message *generate_logon(const unsigned heartbeat_interval, const f8String davi=f8String());
  // bool handle_logout(const unsigned seqnum, const FIX8::Message *msg);
  // Message *generate_logout();
  // bool handle_heartbeat(const unsigned seqnum, const FIX8::Message *msg);
  // Message *generate_heartbeat(const f8String& testReqID);
  // bool handle_resend_request(const unsigned seqnum, const FIX8::Message *msg);
  // Message *generate_resend_request(const unsigned begin, const unsigned end=0);
  // bool handle_sequence_reset(const unsigned seqnum, const FIX8::Message *msg);
  // Message *generate_sequence_reset(const unsigned newseqnum, const bool gapfillflag=false);
  // bool handle_test_request(const unsigned seqnum, const FIX8::Message *msg);
  // Message *generate_test_request(const f8String& testReqID);
  // bool handle_reject(const unsigned seqnum, const FIX8::Message *msg);
  // Message *generate_reject(const unsigned seqnum, const char *what);
  // bool handle_admin(const unsigned seqnum, const FIX8::Message *msg);
  // void modify_outbound(FIX8::Message *msg);
  // bool authenticate(SessionID& id, const FIX8::Message *msg);

  // Override these methods to intercept admin and application methods.
  // bool handle_admin(const unsigned seqnum, const FIX8::Message *msg);

  bool handle_application(const unsigned seqnum, const FIX8::Message *&msg);
};


#endif //IDEFIX_FIX_CLIENT_H
