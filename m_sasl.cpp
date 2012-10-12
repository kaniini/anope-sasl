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

struct SASLSession
{
 Anope::string uid;
};

class SASLMechanism
{
 SASLImplementation *m_impl;
 SASLSession *m_session;

 public:
	SASLMechanism(SASLImplementation *impl, SASLSession *session) : m_impl(impl), m_session(session) { };
	~SASLMechanism();

	virtual SASLReturn MechStart(Anope::string &out);
	virtual SASLReturn MechStep(Anope::string in, Anope::string &out);
};

