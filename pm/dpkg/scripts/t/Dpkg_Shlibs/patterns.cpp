#define EXPORT(x) x {}

namespace NSA {
	class ClassA {
	private:
		class Private {
		public:
			Private();
			virtual ~Private();
			void privmethod1(int);
			void privmethod2(int);
		};
		Private *p;
		class Internal {
		public:
			Internal();
			virtual ~Internal();
			void internal_method1(char);
			void internal_method2(char);
		};
		Internal *i;
	public:
		ClassA();
		virtual ~ClassA();
		virtual void generate_vt(const char *) const;
	};

	EXPORT(ClassA::Private::Private());
	EXPORT(ClassA::Private::~Private());
	EXPORT(void ClassA::Private::privmethod1(int));
	EXPORT(void ClassA::Private::privmethod2(int));

	EXPORT(ClassA::Internal::Internal());
	EXPORT(ClassA::Internal::~Internal());
	EXPORT(void ClassA::Internal::internal_method1(char));
	EXPORT(void ClassA::Internal::internal_method2(char));

	EXPORT(ClassA::ClassA());
	EXPORT(ClassA::~ClassA());
	EXPORT(void ClassA::generate_vt(const char *) const);
};

class ClassB {
public:
	ClassB();
	virtual ~ClassB();
	virtual void generate_vt(const char *) const;
};

EXPORT(ClassB::ClassB());
EXPORT(ClassB::~ClassB());
EXPORT(void ClassB::generate_vt(const char *) const);

class ClassC {
public:
	ClassC();
	virtual ~ClassC();
	virtual void generate_vt(const char *) const;
};

EXPORT(ClassC::ClassC());
EXPORT(ClassC::~ClassC());
EXPORT(void ClassC::generate_vt(const char *) const);

namespace NSB {
	class ClassD : public NSA::ClassA, public ClassB, public ClassC {
	public:
		ClassD();
		virtual ~ClassD() override;
		virtual void generate_vt(const char *) const override;
	};

	EXPORT(ClassD::ClassD());
	EXPORT(ClassD::~ClassD());
	EXPORT(void ClassD::generate_vt(const char *) const);

	class Symver {
	public:
		Symver();
		~Symver();

		void symver_method1();
		void symver_method2();
	};

	EXPORT(Symver::Symver());
	EXPORT(Symver::~Symver());
	EXPORT(void Symver::symver_method1());
	EXPORT(void Symver::symver_method2());

	class SymverOptional {
	public:
		SymverOptional();
		~SymverOptional();
	};

	EXPORT(SymverOptional::SymverOptional());
	EXPORT(SymverOptional::~SymverOptional());
};
