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
#include "fix_client.h"
#include <iostream>


bool fix_client::handle_logon(const unsigned seqnum, const FIX8::Message *msg) {
    std::cout << *msg << std::endl;
    return true;
}

bool fix_client::handle_application(const unsigned seqnum, const FIX8::Message *&msg) {
  return enforce(seqnum, msg) || msg->process(_router);
}
