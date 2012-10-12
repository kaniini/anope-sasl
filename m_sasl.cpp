/*
 * SASL support for Anope.
 * See COPYING.md for copyright and license info.
 */

#include "module.h"

class SASLImplementation
{
 public:
	void SendSVSLOGIN(Anope::string target, Anope::string login);
	void SendSASL(Anope::string target, char mode, Anope::string data);
};

enum SASLReturn
{
 ASASL_FAIL,
 ASASL_MORE,
 ASASL_DONE
};

class SASLMechanism
{
 SASLImplementation *m_impl;
 Anope::string m_uid;

 public:
	SASLMechanism(SASLImplementation *impl, Anope::string uid) : m_impl(impl), m_uid(uid) { };
	~SASLMechanism();

	virtual SASLReturn MechStart(Anope::string &out);
	virtual SASLReturn MechStep(Anope::string in, Anope::string &out);
};

