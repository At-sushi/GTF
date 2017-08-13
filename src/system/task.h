/*!
*	@file
*	@brief �^�X�N(?)�Ǘ��E��`
*/
#pragma once
#include <vector>
#include <string>
#include <map>
#include <list>
#include <unordered_map>
#include <stack>
#include <memory>
#include <functional>
#include <type_traits>



/*!
*	@defgroup Tasks
*	@brief �^�X�N
*
*	CTaskBase���p�������N���X�́A���C�����[�v����Ă΂��X�V�E�`�揈���֐��������Ă��܂��B
*	�V�X�e���͂��̃N���X�̃��X�g�������Ă��܂��B
*	�^�C�g���E�L�����Z���E���� �Ȃǂ̃Q�[���̏�Ԃ̕ύX�́A
*	�����^�X�N�N���X�̐؂�ւ��ɂ���čs���܂��B
*/

namespace GTF
{
    using namespace std;

    /*! 
    *	@ingroup Tasks
    *	@brief	��{�^�X�N
    *
    *	�EExecute��false��Ԃ��Ɣj�������
    *	�E�e�̔r���^�X�N���ύX���ꂽ�Ƃ��A�j�������
    */
    class CTaskBase
    {
    public:
        virtual ~CTaskBase(){}
        virtual void Initialize(){}							//!< Execute�܂���Draw���R�[�������O��1�x�����R�[�������
        virtual bool Execute(double elapsedTime)
                            {return(true);}					//!< ���t���[���R�[�������
        virtual void Terminate(){}							//!< �^�X�N�̃��X�g����O�����Ƃ��ɃR�[�������i���̒���Adelete�����j
        virtual void Draw(){}								//!< �`�掞�ɃR�[�������
        virtual unsigned int GetID() const { return 0; }	//!< 0�ȊO��Ԃ��悤�ɂ����ꍇ�A�}�l�[�W���ɓ���ID�����^�X�N��Add���ꂽ�Ƃ��j�������
        virtual int GetDrawPriority() const { return -1; }	//!< �`��v���C�I���e�B�B�Ⴂ�قǎ�O�Ɂi�㏇�Ɂj�`��B�}�C�i�X�Ȃ�Ε\�����Ȃ�
    };


    /*! 
    *	@ingroup Tasks
    *	@brief �r���^�X�N? =�Q�[���̃V�[���ƍl���Ă��������B
    *
    *	�E���̔r���^�X�N�ƈꏏ�ɂ͓���(Execute)���Ȃ�
    *	�E���̔r���^�X�N���ǉ����ꂽ�ꍇ�AInactivate���R�[������A������false��Ԃ���
    *		�j�������Btrue��Ԃ���Execute�AWndMessage���R�[������Ȃ���ԂɂȂ�A
    *		�V�K�̔r���^�X�N���S�Ĕj�����ꂽ�Ƃ���Activate���Ă΂�A�������ĊJ����B
    *	�E�ʏ�^�X�N�Ƃ̐e�q�֌W�����B
    *	�EAddTask���s��A��xExecute�����s�����܂Œǉ����ۗ������B���̌�ɒǉ����ꂽ�ʏ�^�X�N�͎q�^�X�N�ƂȂ�B
    */
    class CExclusiveTaskBase : public CTaskBase
    {
    public:
        virtual ~CExclusiveTaskBase(){}
        virtual void Activate(unsigned int prvTaskID){}				//!< Execute���ĊJ�����Ƃ��ɌĂ΂��
        virtual bool Inactivate(unsigned int nextTaskID){return true;}//!< ���̔r���^�X�N���J�n�����Ƃ��ɌĂ΂��
    
        virtual int GetDrawPriority() const override {return 0;}				//!< �`��v���C�I���e�B�擾���\�b�h
    };



    /*!
    *	@ingroup Tasks
    *	@brief �풓�^�X�N
    *
    *	�E��{�^�X�N�ƈႢ�A�r���^�X�N���ύX����Ă��j������Ȃ�
    *	�EEnabled�łȂ��Ƃ��ɂ� Execute , WndMessage ���R�[�����Ȃ�
    */
    class CBackgroundTaskBase : public CTaskBase
    {
    public:
        virtual ~CBackgroundTaskBase(){}
        CBackgroundTaskBase(){m_isEnabled=true;}

        bool IsEnabled() const {return m_isEnabled;}
        void Enable(){m_isEnabled = true;}
        void Disable(){m_isEnabled = false;}

    protected:
        bool m_isEnabled;
    };




    /*!
    *	@ingroup System
    *	@brief �^�X�N�Ǘ��N���X
    *
    *	�^�X�N�p���N���X�̃��X�g���Ǘ����A�`��A�X�V�A�E�B���h�E���b�Z�[�W���̔z�M���s���B
    *
    *	���s���ɗ�O���N�������Ƃ��A�ǂ̃N���X����O���N�������̂������O�ɓf���o���B
    *	���̍ۂɎ��s���^��񂩂�N���X�����擾���Ă���̂ŁA�R���p�C���̍ۂɂ�
    *	���s���^���(RTTI�ƕ\�L�����ꍇ������)��ON�ɂ��邱�ƁB
    */

    class CTaskManager
    {
    public:
        CTaskManager();
        ~CTaskManager(){Destroy();}

        using TaskPtr = weak_ptr<CTaskBase>;
        using ExTaskPtr = weak_ptr<CExclusiveTaskBase>;
        using BgTaskPtr = weak_ptr<CBackgroundTaskBase>;

        void Destroy();
            
        //! �ǉ������^�X�N��CTaskManager�����Ŏ����I�ɔj�������̂ŁA�Ăяo������delete���Ȃ����ƁB
        TaskPtr AddTask(CTaskBase *newTask);		        //!< �^�X�N�ǉ�
        ExTaskPtr AddTask(CExclusiveTaskBase *newTask);     //!< �r���^�X�N�ǉ�
        BgTaskPtr AddTask(CBackgroundTaskBase *newTask);    //!< �풓�^�X�N�ǉ�
        void RemoveTaskByID(unsigned int id);				//!< �w��ID�����^�X�N�̏����@�����FExclusive�^�X�N�̓`�F�b�N���Ȃ�
        void RevertExclusiveTaskByID(unsigned int id);		//!< �w��ID�̔r���^�X�N�܂�Terminate/pop����

        //! �ŏ�ʂɂ���G�N�X�N���[�V�u�^�X�N���Q�g
        ExTaskPtr GetTopExclusiveTask() const
        {
            return ex_stack.top().value;
        }

        //!�w��ID�̒ʏ�^�X�N�擾
        TaskPtr FindTask(unsigned int id) const
        {
            return indices.at(id);
        }

        //!�w��ID�̏풓�^�X�N�擾
        BgTaskPtr FindBGTask(unsigned int id) const
        {
           return bg_indices.at(id);
        }

        //! �C�ӂ̃N���X�^�̃^�X�N���擾�i�ʏ�E�풓���p�j
        template<class T> shared_ptr<T> FindTask(unsigned int id) const
        {
            return dynamic_pointer_cast<T>(FindTask_impl<T>(id).lock());
        }

        void Execute(double elapsedTime);					//!< �e�^�X�N��Execute�֐����R�[������
        void Draw();										//!< �e�^�X�N���v���C�I���e�B���ɕ`�悷��

        //!< �r���^�X�N���S���Ȃ��Ȃ�����������ǂ���
        bool ExEmpty() const    {
            return ex_stack.empty();
        }

        //�f�o�b�O
        void DebugOutputTaskList();							//!< ���݃��X�g�ɕێ�����Ă���N���X�̃N���X�����f�o�b�O�o�͂���

    protected:
        using TaskList = list<shared_ptr<CTaskBase>>;
        using BgTaskList = list<shared_ptr<CBackgroundTaskBase>>;
        using DrawPriorityMap = multimap<int, TaskPtr, greater<int>>;

        struct ExTaskInfo {
            const shared_ptr<CExclusiveTaskBase> value;		//!< �r���^�X�N�̃|�C���^
            const TaskList::iterator SubTaskStartPos;		//!< �ˑ�����ʏ�^�X�N�̊J�n�n�_
            DrawPriorityMap drawList;						//!< �`�揇�\�[�g�p�R���e�i

            ExTaskInfo(shared_ptr<CExclusiveTaskBase>& source, TaskList::iterator startPos)
                : value(source), SubTaskStartPos(startPos)
            {
            }
        };
        using ExTaskStack = stack<ExTaskInfo>;

        //! �ʏ�^�X�N��S��Terminate , delete����
        void CleanupAllSubTasks()    {
            CleanupPartialSubTasks(tasks.begin());
        }

        void CleanupPartialSubTasks(TaskList::iterator it_task);	//!< �ꕔ�̒ʏ�^�X�N��Terminate , delete����

        //! ���O�o��
        void OutputLog(string s, ...)
        {
            // Not Implemented
        }

    private:
        template<class T, class = typename enable_if_t<!is_base_of<CBackgroundTaskBase, T>::value>>
            TaskPtr FindTask_impl(unsigned int id) const
        {
            return FindTask(id);
        }
        template<class T, class = typename enable_if_t<is_base_of<CBackgroundTaskBase, T>::value>>
            BgTaskPtr FindTask_impl(unsigned int id) const
        {
            return FindBGTask(id);
        }

        //! �^�X�NExecute
        template<class T, typename I = T::iterator, class QI = deque<I>, typename I_QI = QI::iterator>
            void taskExecute(T& tasks, I i, I ied, double elapsedTime)
        {
            QI deleteList;

            for (; i != ied; ++i){
#ifdef _CATCH_WHILE_EXEC
                try{
#endif
                    if ((*i)->Execute(elapsedTime) == false)
                    {
                        deleteList.push_back(i);
                    }
#ifdef _CATCH_WHILE_EXEC
                }
                catch (...){
                    if (*i == nullptr)OutputLog("catch while execute1 : NULL", SYSLOG_ERROR);
                    else OutputLog("catch while execute1 : %X , %s", *i, typeid(**i).name());
                    break;
                }
#endif
            }

            //�^�X�N��false��Ԃ������̂�����
            I_QI idl = deleteList.begin();
            const I_QI idl_ed = deleteList.end();

            for (; idl != idl_ed; ++idl){
                i = *idl;
                (*i)->Terminate();
                tasks.erase(i);
            }
        }

        TaskList tasks;								//!< ���ݓ��삿�イ�̃^�X�N���X�g
        BgTaskList bg_tasks;						//!< �풓�^�X�N���X�g
        ExTaskStack ex_stack;						//!< �r���^�X�N�̃X�^�b�N�Btop�������s���Ȃ�

        shared_ptr<CExclusiveTaskBase> exNext;		//!< ���݃t���[����Add���ꂽ�r���^�X�N
        DrawPriorityMap drawListBG;					//!< �`�揇�\�[�g�p�R���e�i�i�풓�^�X�N�j
        unordered_map<unsigned int, TaskPtr> indices;
        unordered_map<unsigned int, BgTaskPtr> bg_indices;
    };


}
