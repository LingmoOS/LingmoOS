#define EXPORT(x) x {}

class AngleBrackets {
public:
	AngleBrackets();
	~AngleBrackets();

	void operator>>(int a);
	void operator>>=(int a);

	template<typename T>
	void operator>>(T a);

	template<typename T>
	void operator>>=(T a);
};

EXPORT(AngleBrackets::AngleBrackets())
EXPORT(AngleBrackets::~AngleBrackets())
EXPORT(void AngleBrackets::operator>>(int a))
EXPORT(void AngleBrackets::operator>>=(int a))
template<> void AngleBrackets::operator>><float>(float a) {}
template<> void AngleBrackets::operator>>=<float>(float a) {}
