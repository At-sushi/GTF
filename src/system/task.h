/*!
*	@file
*	@brief �^�X�N(?)�Ǘ��E��`
*/
#pragma once
#include <vector>
#include <string>
#include <list>
#include <unordered_map>
#include <stack>
#include <memory>



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
        virtual int GetDrawPriority() const { return -1; }	//!< �`��v���C�I���e�B�B�Ⴂ�قǎ�O�Ɂi��Ɂj�`��B�}�C�i�X�Ȃ�Ε\�����Ȃ�
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

        bool IsEnabled(){return m_isEnabled;}
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

        typedef std::weak_ptr<CTaskBase> TaskPtr;
        typedef std::weak_ptr<CExclusiveTaskBase> ExTaskPtr;
        typedef std::weak_ptr<CBackgroundTaskBase> BgTaskPtr;

     void Destroy();
            
        //! �ǉ������^�X�N��CTaskManager�����Ŏ����I�ɔj�������̂ŁA�Ăяo������delete���Ȃ����ƁB
        TaskPtr AddTask(CTaskBase *newTask);		        //!< �^�X�N�ǉ�
        ExTaskPtr AddTask(CExclusiveTaskBase *newTask);     //!< �r���^�X�N�ǉ�
        BgTaskPtr AddTask(CBackgroundTaskBase *newTask);    //!< �풓�^�X�N�ǉ�
        void RemoveTaskByID(unsigned int id);				//!< �w��ID�����^�X�N�̏����@�����FExclusive�^�X�N�̓`�F�b�N���Ȃ�
        void RevertExclusiveTaskByID(unsigned int id);		//!< �w��ID�̔r���^�X�N�܂�Terminate/pop����
        ExTaskPtr GetTopExclusiveTask();					//!< �ŏ�ʂɂ���G�N�X�N���[�V�u�^�X�N���Q�g

        //!�w��ID�̏풓�^�X�N�擾
        BgTaskPtr FindBGTask(unsigned int id)
        {
           return bg_indices[id];
        }

        //!�w��ID�̒ʏ�^�X�N�擾
        TaskPtr FindTask(unsigned int id)
        {
            return indices[id];
        }

        //! �C�ӂ̃N���X�^�̒ʏ�^�X�N���擾
        template<class T> std::shared_ptr<T> FindTask(unsigned int id)
        {
            return std::dynamic_pointer_cast<T>(FindTask(id).lock());
        }

        //! �C�ӂ̃N���X�^�̏풓�^�X�N���擾
        template<class T> std::shared_ptr<T> FindBGTask(unsigned int id)
        {
            return std::dynamic_pointer_cast<T>(FindBGTask(id).lock());
        }

        void Execute(double elapsedTime);					//!< �e�^�X�N��Execute�֐����R�[������
        void Draw();										//!< �e�^�X�N���v���C�I���e�B���ɕ`�悷��
        bool ExEmpty();										//!< �r���^�X�N���S���Ȃ��Ȃ�����������ǂ���

        //�f�o�b�O
        void DebugOutputTaskList();							//!< ���݃��X�g�ɕێ�����Ă���N���X�̃N���X�����f�o�b�O�o�͂���

    protected:
        typedef std::list<std::shared_ptr<CTaskBase>> TaskList;
        struct ExTaskInfo {
            const std::shared_ptr<CExclusiveTaskBase> value;		//!< �r���^�X�N�̃|�C���^
            const TaskList::iterator SubTaskStartPos;				//!< �ˑ�����ʏ�^�X�N�̊J�n�n�_

            ExTaskInfo(std::shared_ptr<CExclusiveTaskBase>& source, TaskList::iterator startPos)
                : value(source), SubTaskStartPos(startPos)
            {
            }
        };
        typedef std::stack<ExTaskInfo> ExTaskStack;

        void CleanupAllSubTasks();					//!< �ʏ�^�X�N��S��Terminate , delete����
        void CleanupPartialSubTasks(TaskList::iterator it_task);	//!< �ꕔ�̒ʏ�^�X�N��Terminate , delete����
        void SortTask(TaskList *ptgt);				//!< �^�X�N��`��v���C�I���e�B���ɕ��ׂ�

        void OutputLog(std::string s, ...)			//!< ���O�o��
        {
            // Not Implemented
        }

        static bool CompByDrawPriority(const std::shared_ptr<CTaskBase>& arg1, const std::shared_ptr<CTaskBase>& arg2)	//!< �`��v���C�I���e�B�Ń\�[�g���邽�߂̔�r���Z
        {
            return arg1->GetDrawPriority() > arg2->GetDrawPriority();
        }

    private:
        TaskList tasks;								//!< ���ݓ��삿�イ�̃^�X�N���X�g
        TaskList bg_tasks;							//!< �풓�^�X�N���X�g
        ExTaskStack ex_stack;						//!< �r���^�X�N�̃X�^�b�N�Btop�������s���Ȃ�

        std::shared_ptr<CExclusiveTaskBase> exNext;	//!< ���݃t���[����Add���ꂽ�r���^�X�N
        std::vector<std::shared_ptr<CTaskBase>> tmplist;	//!< �e���|�����p�z��
        std::unordered_map<unsigned int, TaskPtr> indices;
        std::unordered_map<unsigned int, BgTaskPtr> bg_indices;
    };


}
