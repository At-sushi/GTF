

/*============================================================================

    �^�X�N�Ǘ�(?)

==============================================================================*/

#include <cassert>
#include <algorithm>
#include <typeinfo>
#include "task.h"

namespace GTF
{

    CTaskManager::CTaskManager()
    {
        exNext = nullptr;
    }

    void CTaskManager::Destroy()
    {
        TaskList::iterator i, ied;

        //�ʏ�^�X�NTerminate
        i = tasks.begin();
        ied = tasks.end();
        for (; i != ied; i++){
            (*i)->Terminate();
        }
        tasks.clear();

        //�o�b�N�O���E���h�^�X�NTerminate
        i = bg_tasks.begin();
        ied = bg_tasks.end();
        for (; i != ied; i++){
            (*i)->Terminate();
        }
        bg_tasks.clear();

        //�r���^�X�NTerminate
        while (ex_stack.size() != 0){
            ex_stack.top().value->Terminate();
            ex_stack.pop();
        }
    }


    CTaskManager::TaskPtr CTaskManager::AddTask(CTaskBase *newTask)
    {
        if (newTask->GetID() != 0){
            RemoveTaskByID(newTask->GetID());
        }

        CBackgroundTaskBase *pbgt = dynamic_cast<CBackgroundTaskBase*>(newTask);
        if (pbgt){
            //�풓�^�X�N�Ƃ���Add
            return AddTask(pbgt);
        }

        CExclusiveTaskBase *pext = dynamic_cast<CExclusiveTaskBase*>(newTask);
        if (pext){
            //�r���^�X�N�Ƃ���Add
            return AddTask(pext);
        }

        //�ʏ�^�X�N�Ƃ���Add
        tasks.emplace_back(newTask);
        auto& pnew = tasks.back();
        newTask->Initialize();
        if (newTask->GetID() != 0)
            indices[newTask->GetID()] = pnew;
        return pnew;
    }

    CTaskManager::ExTaskPtr CTaskManager::AddTask(CExclusiveTaskBase *newTask)
    {
        if (newTask->GetID() != 0){
            RemoveTaskByID(newTask->GetID());
        }

        //�r���^�X�N�Ƃ���Add
        //Execute����������Ȃ��̂ŁA�|�C���^�ۑ��̂�
        if (exNext){
            OutputLog("��ALERT�� �r���^�X�N��2�ȏ�Add���ꂽ : %s / %s",
                typeid(*exNext).name(), typeid(*newTask).name());
        }
        exNext = std::shared_ptr<CExclusiveTaskBase>(newTask);

        return exNext;
    }

    CTaskManager::BgTaskPtr CTaskManager::AddTask(CBackgroundTaskBase *newTask)
    {
        if (newTask->GetID() != 0){
            RemoveTaskByID(newTask->GetID());
        }

        bg_tasks.emplace_back(newTask);
        // �b��I�Ȍ^�ϊ�
        auto& pbgt = std::static_pointer_cast<CBackgroundTaskBase>(bg_tasks.back());
        assert(std::dynamic_pointer_cast<CBackgroundTaskBase>(pbgt).get());

        //�풓�^�X�N�Ƃ���Add
        pbgt->Initialize();
        if (newTask->GetID() != 0)
            bg_indices[newTask->GetID()] = pbgt;
        return pbgt;
    }

    void CTaskManager::Execute(unsigned int time)
    {
        TaskList::iterator i, ied;
        std::list<TaskList::iterator> deleteList;
        std::list<TaskList::iterator>::iterator idl, idl_ed;

#ifdef ARRAYBOUNDARY_DEBUG
        if(!AfxCheckMemory()){
            OutputLog("AfxCheckMemory() failed");
            return;
        }
#endif

        //�r���^�X�N�Atop�̂�Execute
        if (ex_stack.size() != 0){
            std::shared_ptr<CExclusiveTaskBase> exTsk = ex_stack.top().value;
            bool ex_ret = true;
#ifdef _CATCH_WHILE_EXEC
            try{
#endif
                ex_ret = exTsk->Execute(time);
#ifdef _CATCH_WHILE_EXEC
            }catch(...){
                if (ex_stack.top() == NULL)OutputLog("catch while execute3 : NULL", SYSLOG_ERROR);
                else OutputLog("catch while execute3 : %X %s",ex_stack.top(),typeid(*ex_stack.top()).name());
            }
#endif

            if (!ex_ret)
            {
                if (!exNext){
                    //���ݔr���^�X�N�̕ύX

#ifdef _CATCH_WHILE_EXEC
                    try{
#endif

                        //�ʏ�^�X�N��S�Ĕj������
                        CleanupPartialSubTasks(ex_stack.top().SubTaskStartPos);

#ifdef _CATCH_WHILE_EXEC
                    }catch(...){
                        if ((*i) == NULL)OutputLog("catch while terminate1 : NULL", SYSLOG_ERROR);
                        else OutputLog("catch while terminate1 : %X %s", (*i), typeid(*(*i)).name());
                    }
#endif

                    unsigned int prvID;

#ifdef _CATCH_WHILE_EXEC
                    try{
#endif

                        //���ݔr���^�X�N�̔j��
                        prvID = exTsk->GetID();
                        exTsk->Terminate();
                        exTsk = nullptr;
                        ex_stack.pop();

#ifdef _CATCH_WHILE_EXEC
                    }catch(...){
                        if (exTsk == NULL)OutputLog("catch while terminate2 : NULL", SYSLOG_ERROR);
                        else OutputLog("catch while terminate : %X %s", exTsk, typeid(*exTsk).name());
                    }
#endif


#ifdef _CATCH_WHILE_EXEC
                    try{
#endif

                        //���̔r���^�X�N��Activate����
                        if (ex_stack.size() == 0)return;
                        exTsk = ex_stack.top().value;
                        exTsk->Activate(prvID);

#ifdef _CATCH_WHILE_EXEC
                    }catch(...){
                        if (exTsk == NULL)OutputLog("catch while activate : NULL", SYSLOG_ERROR);
                        else OutputLog("catch while activate : %X %s", exTsk, typeid(*exTsk).name());
                    }
#endif


                    return;
                }
            }
        }

        //�ʏ�^�X�NExecute
        i = ex_stack.empty() ? tasks.begin() : ex_stack.top().SubTaskStartPos;
        ied = tasks.end();
        for (; i != ied; i++){
#ifdef _CATCH_WHILE_EXEC
            try{
#endif
                if ((*i)->Execute(time) == false)
                {
                    deleteList.push_back(i);
                }
#ifdef _CATCH_WHILE_EXEC
            }catch(...){
                if(*i==NULL)OutputLog("catch while execute1 : NULL",SYSLOG_ERROR);
                else OutputLog("catch while execute1 : %X , %s",*i,typeid(**i).name());
                break;
            }
#endif
        }
        //�ʏ�^�X�N��false��Ԃ������̂�����
        if (deleteList.size() != 0){
            idl = deleteList.begin();
            idl_ed = deleteList.end();
            for (; idl != idl_ed; idl++){
                i = *idl;
                (*i)->Terminate();
                tasks.erase(i);
            }
            deleteList.clear();
        }

        //�풓�^�X�NExecute
        i = bg_tasks.begin();
        ied = bg_tasks.end();
        for (; i != ied; i++)
        {
#ifdef _CATCH_WHILE_EXEC
            try{
#endif
                if ((*i)->Execute(time) == false){
                    deleteList.push_back(i);
                }
#ifdef _CATCH_WHILE_EXEC
            }catch(...){
                if(*i==NULL)OutputLog("catch while execute2 : NULL",SYSLOG_ERROR);
                else OutputLog("catch while execute2 : %X %s",*i,typeid(**i).name());
            }
#endif
        }
        //�풓�^�X�N��false��Ԃ������̂�����
        if (deleteList.size() != 0){
            idl = deleteList.begin();
            idl_ed = deleteList.end();
            for (; idl != idl_ed; idl++){
                i = *idl;
                (*i)->Terminate();
                bg_tasks.erase(i);
            }
            deleteList.clear();
        }

        // �V�����^�X�N������ꍇ
        if (exNext){
            //���ݔr���^�X�N��Inactivate
            if (ex_stack.size() != 0){
                auto& exTsk = ex_stack.top().value;
                if (!exTsk->Inactivate(exNext->GetID())){
                    //�ʏ�^�X�N��S�Ĕj������
                    CleanupPartialSubTasks(ex_stack.top().SubTaskStartPos);

                    exTsk->Terminate();
                    ex_stack.pop();
                }
            }

            //Add���ꂽ�^�X�N��Initialize���ē˂�����
            i = tasks.emplace(tasks.end(), new CTaskBase());				// �_�~�[�^�X�N�}��
            ex_stack.emplace(std::move(exNext), i);
            ex_stack.top().value->Initialize();

            exNext = nullptr;
        }
    }


    void CTaskManager::Draw()
    {
        TaskList::iterator i, ied;

        assert(tmplist.empty());
        tmplist.reserve(tasks.size());

        //�ʏ�^�X�NDraw
        i = tasks.begin();
        ied = tasks.end();
        for (; i != ied; i++){
            if ((*i)->GetDrawPriority() >= 0){
                tmplist.push_back(*i);
            }
        }

        //�o�b�N�O���E���h�^�X�NDraw
        i = bg_tasks.begin();
        ied = bg_tasks.end();
        for (; i != ied; i++){
            if ((*i)->GetDrawPriority() >= 0){
                tmplist.push_back(*i);
            }
        }

        //�r���^�X�NDraw
        if (ex_stack.size() != 0){
            if (ex_stack.top().value->GetDrawPriority() >= 0){
                tmplist.push_back(ex_stack.top().value);
            }
        }

        std::sort(tmplist.begin(), tmplist.end(), CompByDrawPriority);//�`��v���C�I���e�B���Ƀ\�[�g

        //�`��
        auto iv = tmplist.begin();
        auto iedv = tmplist.end();
        for (; iv != iedv; iv++)
        {
#ifdef _CATCH_WHILE_RENDER
            try{
#endif
                (*iv)->Draw();
#ifdef _CATCH_WHILE_RENDER
            }catch(...){
                OutputLog("catch while draw : %X %s", *iv, typeid(**iv).name());
            }
#endif
        }

        // �ꎞ���X�g�j��
        tmplist.clear();
    }

    void CTaskManager::RemoveTaskByID(unsigned int id)
    {
        TaskList::iterator i, ied;

        //�ʏ�^�X�N���`�F�b�N
        if (indices.find(id) != indices.end())
        {
            i = tasks.begin();
            ied = tasks.end();
            for (; i != ied; i++){
                if (id == (*i)->GetID()){
                    (*i)->Terminate();
                    tasks.erase(i);
                    return;
                }
            }
        }

        //�o�b�N�O���E���h�^�X�NTerminate
        if (bg_indices.find(id) != bg_indices.end())
        {
            i = bg_tasks.begin();
            ied = bg_tasks.end();
            for (; i != ied; i++){
                if (id == (*i)->GetID()){
                    (*i)->Terminate();
                    bg_tasks.erase(i);
                    return;
                }
            }
        }
    }


    //�ŏ�ʂɂ���G�N�X�N���[�V�u�^�X�N���Q�g
    CTaskManager::ExTaskPtr CTaskManager::GetTopExclusiveTask()
    {
        if (ex_stack.size() == 0)return ExTaskPtr();
        return ex_stack.top().value;
    }

    //�w��ID�̔r���^�X�N�܂�Terminate/pop����
    void CTaskManager::ReturnExclusiveTaskByID(unsigned int id)
    {
        bool act = false;
        unsigned int previd = 0;

        while (ex_stack.size() != 0){
            const std::shared_ptr<CExclusiveTaskBase>& task = ex_stack.top().value;
            if (task->GetID() == id){
                if (act){
                    task->Activate(previd);
                }
                return;
            }
            else{
                previd = task->GetID();
                act = true;
                CleanupPartialSubTasks(ex_stack.top().SubTaskStartPos);
                task->Terminate();
                ex_stack.pop();
            }
        }
    }

    //�ʏ�^�X�N��S�Ĕj������
    void CTaskManager::CleanupAllSubTasks()
    {
        CleanupPartialSubTasks(tasks.begin());
    }

    //�ʏ�^�X�N���ꕔ�����j������
    void CTaskManager::CleanupPartialSubTasks(TaskList::iterator it_task)
    {
        TaskList::iterator i, ied;

        i = it_task;
        ied = tasks.end();
        for (; i != ied; i++){
            std::shared_ptr<CTaskBase>& delTgt = (*i);
            delTgt->Terminate();
        }
        tasks.erase(it_task, ied);
    }


    //�f�o�b�O�E�^�X�N�ꗗ�\��
    void CTaskManager::DebugOutputTaskList()
    {
        OutputLog("\n\n��CTaskManager::DebugOutputTaskList() - start");

        TaskList::iterator i, ied;

        OutputLog("���ʏ�^�X�N�ꗗ��");
        //�ʏ�^�X�N
        i = tasks.begin();
        ied = tasks.end();
        for (; i != ied; i++){
            OutputLog(typeid(**i).name());
        }

        OutputLog("���풓�^�X�N�ꗗ��");
        //�o�b�N�O���E���h�^�X�N
        i = bg_tasks.begin();
        ied = bg_tasks.end();
        for (; i != ied; i++){
            OutputLog(typeid(**i).name());
        }

        //�r���^�X�N	
        OutputLog("\n");
        OutputLog("�����݂̃^�X�N�F");
        if (ex_stack.empty())
            OutputLog("�Ȃ�");
        else
            OutputLog(typeid(*ex_stack.top().value).name());


        OutputLog("\n\n��CTaskManager::DebugOutputTaskList() - end\n\n");
    }

    //�S���Ȃ��Ȃ����������A��΂�������
    bool CTaskManager::ExEmpty()
    {
        return (ex_stack.size() == 0) ? true : false;
    }

}
