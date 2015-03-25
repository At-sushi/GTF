/*!
*	@file
*	@brief �^�X�N(?)�Ǘ��E��`
*/
#pragma once
#include <string>
#include <list>
#include <stack>
#include <memory>



/*!
*	@defgroup Tasks
*	@brief �^�X�N? =�Q�[���̃V�[���ƍl���Ă��������B
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
    *	�E�r���^�X�N���ύX���ꂽ�Ƃ��A�j�������
    */
    class CTaskBase
    {
    public:
        virtual ~CTaskBase(){}
        virtual void Initialize(){}					//!< Execute�܂���Draw���R�[�������O��1�x�����R�[�������
        virtual bool Execute(unsigned int time)
                            {return(true);}			//!< ���t���[���R�[�������
        virtual void Terminate(){}					//!< �^�X�N�̃��X�g����O�����Ƃ��ɃR�[�������i���̒���Adelete�����j
        virtual void Draw(){}						//!< �`�掞�ɃR�[�������
        virtual unsigned int GetID(){return 0;}			//!< 0�ȊO��Ԃ��悤�ɂ����ꍇ�A�}�l�[�W���ɓ���ID�����^�X�N��Add���ꂽ�Ƃ��j�������
        virtual int GetDrawPriority(){return -1;}	//!< �`��v���C�I���e�B�B�Ⴂ�قǎ�O�Ɂi��Ɂj�`��B�}�C�i�X�Ȃ�Ε\�����Ȃ�
    };


    /*! 
    *	@ingroup Tasks
    *	@brief �r���^�X�N
    *
    *	�E���̔r���^�X�N�ƈꏏ�ɂ͓���(Execute)���Ȃ�
    *	�E���̔r���^�X�N���ǉ����ꂽ�ꍇ�AInactivate���R�[������A������false��Ԃ���
    *		�j�������Btrue��Ԃ���Execute�AWndMessage���R�[������Ȃ���ԂɂȂ�A
    *		�V�K�̔r���^�X�N���S�Ĕj�����ꂽ�Ƃ���Activate���Ă΂�A�������ĊJ����B
    */
    class CExclusiveTaskBase : public CTaskBase
    {
    public:
        virtual ~CExclusiveTaskBase(){}
        virtual void Activate(unsigned int prvTaskID){}				//!< Execute���ĊJ�����Ƃ��ɌĂ΂��
        virtual bool Inactivate(unsigned int nextTaskID){return false;}//!< ���̔r���^�X�N���J�n�����Ƃ��ɌĂ΂��
    
        virtual int GetDrawPriority(){return 0;}				//!< �`��v���C�I���e�B�擾���\�b�h
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
            
        //! �^�X�N�ǉ�
        //! �ǉ������^�X�N��CTaskManager�����Ŏ����I�ɔj�������̂ŁA�Ăяo������delete���Ȃ����ƁB
        TaskPtr AddTask(CTaskBase *newTask);
        //! �r���^�X�N�ǉ�
        //! �ǉ������^�X�N��CTaskManager�����Ŏ����I�ɔj�������̂ŁA�Ăяo������delete���Ȃ����ƁB
        ExTaskPtr AddTask(CExclusiveTaskBase *newTask);
        //! �풓�^�X�N�ǉ�
        //! �ǉ������^�X�N��CTaskManager�����Ŏ����I�ɔj�������̂ŁA�Ăяo������delete���Ȃ����ƁB
        BgTaskPtr AddTask(CBackgroundTaskBase *newTask);
        void RemoveTaskByID(unsigned int id);				//!< �w��ID�����^�X�N�̏����@�����FExclusive�^�X�N�̓`�F�b�N���Ȃ�
        void ReturnExclusiveTaskByID(unsigned int id);		//!< �w��ID�̔r���^�X�N�܂�Terminate/pop����
        ExTaskPtr GetTopExclusiveTask();			//!< �ŏ�ʂɂ���G�N�X�N���[�V�u�^�X�N���Q�g
        BgTaskPtr FindBGTask(unsigned int id);	//!< �w��ID�����풓�^�X�N�Q�b�g
        TaskPtr FindTask(unsigned int id);				//!< �w��ID�����ʏ�^�X�N�Q�b�g

        void Execute(unsigned int time);					//!< �e�^�X�N��Execute�֐����R�[������
        void Draw();										//!< �e�^�X�N���v���C�I���e�B���ɕ`�悷��
        bool ExEmpty();										//!< �r���^�X�N���S���Ȃ��Ȃ�����������ǂ���

        //�f�o�b�O
        void DebugOutputTaskList();							//!< ���݃��X�g�ɕێ�����Ă���N���X�̃N���X�����f�o�b�O�o�͂���

    protected:
        typedef std::list<std::shared_ptr<CTaskBase>> TaskList;
        typedef std::stack<std::shared_ptr<CExclusiveTaskBase>> ExTaskStack;

        void CleanupAllSubTasks();					//!< �ʏ�^�X�N��S��Terminate , delete����
        void SortTask(TaskList *ptgt);				//!< �^�X�N��`��v���C�I���e�B���ɕ��ׂ�

        void OutputLog(std::string s, ...)			//!< ���O�o��
        {
            // Not Implemented
        }

    private:
        static bool CompByDrawPriority(std::shared_ptr<CTaskBase> arg1, std::shared_ptr<CTaskBase> arg2)	//!< �`��v���C�I���e�B�Ń\�[�g���邽�߂̔�r���Z
        {
            return arg1->GetDrawPriority() > arg2->GetDrawPriority();
        }

        TaskList tasks;								//!< ���ݓ��삿�イ�̃^�X�N���X�g
        TaskList bg_tasks;							//!< �풓�^�X�N���X�g
        ExTaskStack ex_stack;						//!< �r���^�X�N�̃X�^�b�N�Btop�������s���Ȃ�

        std::shared_ptr<CExclusiveTaskBase> exNext;	//!< ���݃t���[����Add���ꂽ�r���^�X�N
    };


}
