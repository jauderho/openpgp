/*
  Copyright 2019 SoloKeys Developers

  Licensed under the Apache License, Version 2.0, <LICENSE-APACHE or
  http://apache.org/licenses/LICENSE-2.0> or the MIT license <LICENSE-MIT or
  http://opensource.org/licenses/MIT>, at your option. This file may not be
  copied, modified, or distributed except according to those terms.
 */

#ifndef SRC_APPLETS_OPENPGPAPPLET_H_
#define SRC_APPLETS_OPENPGPAPPLET_H_

#include "applet.h"

namespace Applet {

	class OpenPGPApplet: Applet {
		// TODO: applet state. INIT/WORK. save/load to file
	private:
		// OpenPGP AID
		const bstr aid = {0xd2, 0x76, 0x00, 0x01, 0x24, 0x01};
	public:

	};

}


#endif /* SRC_APPLETS_OPENPGPAPPLET_H_ */
