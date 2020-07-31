namespace Systems
{
	class StateController
	{
	public:
		StateController();
		~StateController();

		void initializationFinalize() const;
		void frameSetup() const;
		void frameTeardown() const;
	};
}
