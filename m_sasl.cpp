/*
 * SASL support for Anope.
 * See COPYING.md for copyright and license info.
 */

#include "module.h"

class SASLMechanism;

std::map<Anope::string, SASLMechanism *> sasl_sessions;

class SASLImplementation
{
 protected:
	Module *m_module;
	SASLImplementation(Module *module) : m_module(module) { };

 public:
	virtual void SendSVSLOGIN(Anope::string target, Anope::string login);
	virtual void SendSASL(Anope::string target, char mode, Anope::string data);
	void HandleMessage(Anope::string uid, char mode, Anope::string data);
	void DestroySession(Anope::string uid);
	SASLMechanism *FindSession(Anope::string uid, Anope::string mech);
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
	SASLIdentifyRequest *m_idreq;
	SASLImplementation *m_impl;
	Anope::string m_uid;
	Module *m_module;

 public:
	SASLMechanism(Module *m, SASLImplementation *impl, Anope::string uid) : m_impl(impl), m_uid(uid), m_module(m) { };
	virtual ~SASLMechanism();

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
		m_impl->DestroySession(m_uid);
	}

	void OnFailedAuth()
	{
		m_impl->SendSASL(m_uid, 'D', "F");
		m_impl->DestroySession(m_uid);
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

struct PlainMechanism : public SASLMechanism
{
	PlainMechanism(Module *module, SASLImplementation *impl, Anope::string uid) : SASLMechanism(module, impl, uid) { };

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

SASLMechanism *SASLImplementation::FindSession(Anope::string uid, Anope::string mech)
{
	SASLMechanism *mech_p = sasl_sessions[uid];

	if (mech_p)
		return mech_p;

	mech_p = new PlainMechanism(m_module, this, uid);
	mech_p->MechStart();
	sasl_sessions[uid] = mech_p;

	return mech_p;
}

void SASLImplementation::HandleMessage(Anope::string uid, char mode, Anope::string data)
{
	SASLMechanism *mech = FindSession(uid, "PLAIN");

	mech->MechStep(data);
}

void SASLImplementation::DestroySession(Anope::string uid)
{
	SASLMechanism *mech_p = sasl_sessions[uid];

	if (mech_p)
		delete mech_p;

	sasl_sessions.erase(uid);
}

class UnrealSASLImplementation : public SASLImplementation
{
	struct IRCDMessageSASL : IRCDMessage
	{
		SASLImplementation *m_parent;

		IRCDMessageSASL(SASLImplementation *parent) : IRCDMessage("SASL", 3), m_parent(parent) { };

		/*
		 * params[0] = uid
		 * params[1] = mode
		 * params[2] = data
		 */
		bool Run(MessageSource &source, const std::vector<Anope::string> &params)
		{
			m_parent->HandleMessage(params[0], params[1][0], params[2]);
			return true;
		}
	};

	IRCDMessageSASL *sasl_message;

 public:
	UnrealSASLImplementation(Module *module) : SASLImplementation(module)
	{
		/* XXX: memory leak on module unload... */
		sasl_message = new IRCDMessageSASL(this);
	}

	void SendSVSLOGIN(Anope::string target, Anope::string login)
	{
		Anope::string server;
		size_t pos;

		/* convert PUID into servername */
		pos = target.find('!');
		server = target.substr(0, pos);

		UplinkSocket::Message(Me) << "SVSLOGIN " << server << " " << target << " " << login;
	}

	void SendSASL(Anope::string target, char mode, Anope::string data)
	{
		Anope::string server;
		size_t pos;

		/* convert PUID into servername */
		pos = target.find('!');
		server = target.substr(0, pos);

		UplinkSocket::Message(Me) << "SASL " << server << " " << target << " " << mode << " " << data;
	}
};

class InspIRCdSASLImplementation : public SASLImplementation
{
	struct IRCDMessageENCAP : IRCDMessage
	{
		SASLImplementation *m_parent;

		IRCDMessageENCAP(SASLImplementation *parent) : IRCDMessage("ENCAP", 2), m_parent(parent) { };

		/*
		 * On InspIRCd, like Unreal, the message data is encapsulated, so we have
		 * to de-encapsulate it.
		 *
		 * params[0] = server mask
		 * params[1] = command
		 * params[2+] = payload
		 */
		bool Run(MessageSource &source, const std::vector<Anope::string> &params)
		{
			if (params[1] != "SASL")
				return false;

			m_parent->HandleMessage(params[2], params[4][0], params[5]);
			return true;
		}
	};

	IRCDMessageENCAP *encap_message;

 public:
	InspIRCdSASLImplementation(Module *module) : SASLImplementation(module)
	{
		/* XXX: memory leak on module unload... */
		encap_message = new IRCDMessageENCAP(this);
	}

	void SendSVSLOGIN(Anope::string target, Anope::string login)
	{
		UplinkSocket::Message(Me) << "METADATA " << target << " accountname :" << login;
	}

	void SendSASL(Anope::string target, char mode, Anope::string data)
	{
		Anope::string sid = target.substr(0, 3);

		UplinkSocket::Message(Me) << "ENCAP " << sid << " SASL " << Me->GetSID() << " " << target << " " << mode << " " << data;
	}
};

class SASLModule : public Module
{
 private:
	SASLImplementation *impl;

 public:
	SASLModule(const Anope::string &name, const Anope::string &creator) : Module(name, creator, THIRD)
	{
		Anope::string protoname;

		this->SetAuthor("atheme.org");

		protoname = ircdproto->GetProtocolName();
		if (protoname == "UnrealIRCd 3.2.x")
			impl = new UnrealSASLImplementation(this);
		else if (protoname == "InspIRCd 2.0" || protoname == "InspIRCd 1.2")
			impl = new InspIRCdSASLImplementation(this);
		else
			Log() << "Protocol '" << protoname << "' is not yet supported by this module. :(";
	}
};

MODULE_INIT(SASLModule)
