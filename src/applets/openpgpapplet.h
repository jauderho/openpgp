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
#include "openpgp/openpgpfactory.h"
#include "openpgp/openpgpconst.h"

namespace Applet {

struct OpenPGPAppletState {
	bool pw1Authenticated;
	bool pw3Authenticated;
};

struct OpenPGPAppletConfig {
	OpenPGP::LifeCycleState state;
	bstr pw1;
	bstr pw3;
	Util::Error Load();
	Util::Error Save();
};

class OpenPGPApplet: public Applet {
	// TODO: applet state. INIT/WORK. save/load to file
	OpenPGPAppletState state;
	OpenPGPAppletConfig config;

private:
	// OpenPGP AID
	const bstr aid = "\xd2\x76\x00\x01\x24\x01"_bstr;
public:
	OpenPGPApplet();

	virtual const bstr *GetAID();

	virtual Util::Error APDUExchange(bstr apdu, bstr &result);

	void ClearAuth(OpenPGP::Password passwdId);
	void SetAuth(OpenPGP::Password passwdId);
};

} // namespace Applet


#endif /* SRC_APPLETS_OPENPGPAPPLET_H_ */
