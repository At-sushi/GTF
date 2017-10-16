#include "stdafx.h"
#include "CppUnitTest.h"
#include "../../../src/system/task.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace GTF;
#include <vector>

static std::vector<int> veve;
template<typename T, class B>
class CTekitou2 : public B
{
public:
	CTekitou2(T init) : hogehoge(init)
	{

	}
	~CTekitou2()
	{}

	T hogehoge;

	bool Execute(double e) override{ veve.push_back(hogehoge); return true; }
	unsigned int GetID()const override{ return hogehoge; }
};

namespace GTFTest
{
	TEST_CLASS(UnitTest1)
	{
	public:

		template<typename T, class B>
		class CTekitou : public B
		{
		public:
			CTekitou(T init) : hogehoge(init)
			{

			}
			~CTekitou()
			{}

			T hogehoge;
			unsigned int GetID()const{ return hogehoge; }
			int GetDrawPriority()const{ return 1; }
			void Draw() override{ veve.push_back(hogehoge); }
		};

		TEST_METHOD(TestMethod1)
		{
			// TODO: �e�X�g �R�[�h�������ɑ}�����܂�
			CTaskManager task;

			auto ptr = task.AddNewTask< CTekitou<int, CTaskBase> >(1).lock();
			Assert::AreEqual((void*)task.FindTask(ptr->GetID()).lock().get(), (void*)ptr.get());
			Assert::AreEqual((void*)task.FindTask<CTaskBase>(ptr->GetID()).get(), (void*)ptr.get());
		}

		TEST_METHOD(TestMethod2)
		{
			// TODO: �e�X�g �R�[�h�������ɑ}�����܂�
			CTaskManager task;

			auto ptr = task.AddTask(new CTekitou<int, CBackgroundTaskBase>(1)).lock();
			Assert::AreEqual((void*)(task.FindTask<CBackgroundTaskBase>(ptr->GetID())).get(), (void*)ptr.get());
			auto ptr2 = task.AddTask(static_cast<CTaskBase*>(new CTekitou<int, CBackgroundTaskBase>(1))).lock();
			Assert::AreNotEqual((void*)task.FindTask(ptr2->GetID()).lock().get(), (void*)ptr2.get());
			Assert::AreEqual((void*)task.FindBGTask(ptr2->GetID()).lock().get(), (void*)ptr2.get());
		}

		TEST_METHOD(TestMethod3)
		{
			// TODO: �e�X�g �R�[�h�������ɑ}�����܂�
			CTaskManager task;

			auto ptr = task.AddTask(new CTekitou<int, CExclusiveTaskBase>(1)).lock();
			Assert::AreNotEqual((void*)(task.FindTask<CExclusiveTaskBase>(ptr->GetID())).get(), (void*)ptr.get());
			auto ptr2 = task.AddTask(static_cast<CTaskBase*>(new CTekitou<int, CExclusiveTaskBase>(1))).lock();
			Assert::AreNotEqual((void*)task.FindTask(ptr2->GetID()).lock().get(), (void*)ptr2.get());
		}

		TEST_METHOD(���s����)
		{
			// TODO: �e�X�g �R�[�h�������ɑ}�����܂�
			CTaskManager task;

			veve.clear();
			auto ptr = task.AddNewTask< CTekitou2<int, CExclusiveTaskBase> >(1);
			auto ptr2 = task.AddNewTask< CTekitou2<int, CTaskBase> >(2);
			task.Execute(0);
			Assert::AreEqual(2, veve[0]);
		}

		TEST_METHOD(���s����2)
		{
			// TODO: �e�X�g �R�[�h�������ɑ}�����܂�
			static CTaskManager task;
			class ct : public CTekitou2 < int, CExclusiveTaskBase >
			{
			public:
				ct(int init) : CTekitou2 < int, CExclusiveTaskBase >(init)
				{

				}
				void Initialize()
				{
					task.AddTask(new CTekitou2<int, CTaskBase>(2));
				}
			};

			veve.clear();
			auto ptr = task.AddNewTask<ct>(1);
			task.Execute(0);
			task.Execute(1);
			Assert::AreEqual(1, veve[0]);
			Assert::AreEqual(2, veve[1]);
		}

		TEST_METHOD(�`��)
		{
			// TODO: �e�X�g �R�[�h�������ɑ}�����܂�
			static CTaskManager task;
			class ct2 : public CTekitou2 < int, CExclusiveTaskBase >
			{
			public:
				ct2(int init) : CTekitou2 < int, CExclusiveTaskBase >(init)
				{

				}
				void Initialize()
				{
					task.AddNewTask< CTekitou<int, CTaskBase> >(hogehoge + 1);
				}
			};

			for (int i = 1; i < 257; i++)
			{
				task.AddNewTask<ct2>(i * 2);
				task.Execute(0);
			}
			veve.clear();
			task.RemoveTaskByID(1);
			for (int i = 0; i < 256; i++)
				task.Draw();

			Assert::AreEqual(513, veve[0]);
		}

		TEST_METHOD(�`��2)
		{
			// TODO: �e�X�g �R�[�h�������ɑ}�����܂�
			static CTaskManager task;
			class ct2 : public CTekitou2 < int, CExclusiveTaskBase >
			{
			public:
				ct2(int init) : CTekitou2 < int, CExclusiveTaskBase >(init)
				{

				}
				void Initialize()
				{
					task.AddNewTask< CTekitou<int, CBackgroundTaskBase> >(hogehoge + 1);
				}
			};

			for (int i = 1; i < 257; i++)
			{
				task.AddNewTask<ct2>(i * 2);
				task.Execute(0);
			}
			veve.clear();
			task.RemoveTaskByID(1);
			for (int i = 0; i < 256; i++)
				task.Draw();

			Assert::AreEqual(3, veve[0]);
		}

		TEST_METHOD(�^�X�N�̈ˑ��֌W)
		{
			// TODO: �e�X�g �R�[�h�������ɑ}�����܂�
			static CTaskManager task;
			class ct : public CTekitou2 < int, CExclusiveTaskBase >
			{
			public:
				ct(int init) : CTekitou2 < int, CExclusiveTaskBase >(init)
				{

				}
				void Initialize()
				{
					task.AddNewTask< CTekitou2<int, CTaskBase> >(hogehoge + 20);
				}
				virtual bool Inactivate(unsigned int nextTaskID){ return true; }//!< ���̔r���^�X�N���J�n�����Ƃ��ɌĂ΂��
			};

			veve.clear();
			auto ptr = task.AddNewTask<ct>(1);
			task.Execute(0);
			auto ptr2 = task.AddNewTask<ct>(3);
			task.Execute(1);
			Assert::AreEqual(1, veve[0]);
			Assert::AreEqual(1 + 20, veve[1]);
			task.Execute(2);
			Assert::AreEqual(3, veve[2]);
			Assert::AreEqual(3 + 20, veve[3]);
			task.RevertExclusiveTaskByID(1);
			task.Execute(3);
			Assert::AreEqual(1, veve[4]);
			Assert::AreEqual(1 + 20, veve[5]);
			ptr2 = task.AddNewTask<ct>(4);
			task.Execute(4);
			task.RemoveTaskByID(21);
			task.Execute(5);
			Assert::AreEqual(4, veve[8]);
			Assert::AreEqual(4 + 20, veve[9]);

		}

	};
}