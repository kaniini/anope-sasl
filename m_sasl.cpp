/*
 * SASL support for Anope.
 * See COPYING.md for copyright and license info.
 */

#include "module.h"

std::set<Anope::string> sasl_sessions;

class SASLImplementation
{
 public:
	void SendSVSLOGIN(Anope::string target, Anope::string login);
	void SendSASL(Anope::string target, char mode, Anope::string data);
};

class SASLMechanism;
class SASLIdentifyRequest : public IdentifyRequest
{
 private:
	SASLMechanism *m_mech;

 public:
	SASLIdentifyRequest(Module *m, SASLMechanism *mech, Anope::string account, Anope::string password) : IdentifyRequest(m, account, password), m_mech(mech) { };
	void OnSuccess();
	void OnFail();
};

class SASLMechanism
{
 protected:
	Module *m_module;
	SASLImplementation *m_impl;
	SASLIdentifyRequest *m_idreq;
	Anope::string m_uid;

 public:
	SASLMechanism(Module *m, SASLImplementation *impl, Anope::string uid) : m_impl(impl), m_uid(uid), m_module(m) { };
	~SASLMechanism();

	virtual void MechStart();
	virtual void MechStep(Anope::string in);

	void VerifyPassword(Anope::string account, Anope::string password)
	{
		SASLIdentifyRequest *req = new SASLIdentifyRequest(m_module, this, account, password);
		FOREACH_MOD(I_OnCheckAuthentication, OnCheckAuthentication(NULL, req));
		req->Dispatch();
	}

	void OnSuccessfulAuth(Anope::string account)
	{
		m_impl->SendSVSLOGIN(m_uid, account);
		m_impl->SendSASL(m_uid, 'D', "S");
	}

	void OnFailedAuth()
	{
		m_impl->SendSASL(m_uid, 'D', "F");
	}
};

void SASLIdentifyRequest::OnSuccess()
{
	m_mech->OnSuccessfulAuth(GetAccount());
}

void SASLIdentifyRequest::OnFail()
{
	m_mech->OnFailedAuth();
}

class PlainMechanism : public SASLMechanism
{
 public:
	void MechStart()
	{
		m_impl->SendSASL(m_uid, 'C', "+");
	}

	void MechStep(Anope::string in)
	{
		Anope::string authzid, authcid, password;
		Anope::string decoded, next;
		size_t pos;

		Anope::B64Decode(in, decoded);

		pos = decoded.find('\0');
		authzid = decoded.substr(0, pos);

		/* avoid undefined behaviour */
		next = decoded.substr(pos);
		decoded = next;

		pos = decoded.find('\0');
		authcid = decoded.substr(0, pos);
		password = decoded.substr(pos);

		/* verify password */
		VerifyPassword(authcid, password);
	}
};
