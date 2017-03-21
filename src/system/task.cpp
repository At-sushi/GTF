

/*============================================================================

    �^�X�N�Ǘ�(?)

==============================================================================*/

#include <cassert>
#include <algorithm>
#include <typeinfo>
#include "task.h"

namespace GTF
{
    using namespace std;

    CTaskManager::CTaskManager()
    {
        exNext = nullptr;

        // �_�~�[�f�[�^�}��
        const auto it = tasks.emplace(tasks.end(), make_shared<CTaskBase>());
        ex_stack.emplace(exNext, it);
    }

    void CTaskManager::Destroy()
    {
        TaskList::iterator i, ied;

        //�ʏ�^�X�NTerminate
        i = tasks.begin();
        ied = tasks.end();
        for (; i != ied; ++i){
            (*i)->Terminate();
        }
        tasks.clear();

        //�o�b�N�O���E���h�^�X�NTerminate
        i = bg_tasks.begin();
        ied = bg_tasks.end();
        for (; i != ied; ++i){
            (*i)->Terminate();
        }
        bg_tasks.clear();

        //�r���^�X�NTerminate
        while (ex_stack.size() != 0 && ex_stack.top().value){
            ex_stack.top().value->Terminate();
            ex_stack.pop();
        }
    }


    CTaskManager::TaskPtr CTaskManager::AddTask(CTaskBase *newTask)
    {
        assert(newTask);

        CExclusiveTaskBase *pext = dynamic_cast<CExclusiveTaskBase*>(newTask);
        if (pext){
            //�r���^�X�N�Ƃ���Add
            return AddTask(pext);
        }

        if (newTask->GetID() != 0){
            RemoveTaskByID(newTask->GetID());
        }

        CBackgroundTaskBase *pbgt = dynamic_cast<CBackgroundTaskBase*>(newTask);
        if (pbgt){
            //�풓�^�X�N�Ƃ���Add
            return AddTask(pbgt);
        }

        //�ʏ�^�X�N�Ƃ���Add
        tasks.emplace_back(newTask);
        auto& pnew = tasks.back();
        newTask->Initialize();
        if (newTask->GetID() != 0)
            indices[newTask->GetID()] = pnew;
        if (pnew->GetDrawPriority() >= 0)
            ex_stack.top().drawList.emplace(pnew->GetDrawPriority(), pnew);
        return pnew;
    }

    CTaskManager::ExTaskPtr CTaskManager::AddTask(CExclusiveTaskBase *newTask)
    {
        //�r���^�X�N�Ƃ���Add
        //Execute����������Ȃ��̂ŁA�|�C���^�ۑ��̂�
        if (exNext){
            OutputLog("��ALERT�� �r���^�X�N��2�ȏ�Add���ꂽ : %s / %s",
                typeid(*exNext).name(), typeid(*newTask).name());
        }
        exNext = shared_ptr<CExclusiveTaskBase>(newTask);

        return exNext;
    }

    CTaskManager::BgTaskPtr CTaskManager::AddTask(CBackgroundTaskBase *newTask)
    {
        if (newTask->GetID() != 0){
            RemoveTaskByID(newTask->GetID());
        }

        bg_tasks.emplace_back(newTask);
        // �b��I�Ȍ^�ϊ�
        auto& pbgt = static_pointer_cast<CBackgroundTaskBase>(bg_tasks.back());
        assert(dynamic_pointer_cast<CBackgroundTaskBase>(pbgt).get());

        //�풓�^�X�N�Ƃ���Add
        pbgt->Initialize();
        if (newTask->GetID() != 0)
            bg_indices[newTask->GetID()] = pbgt;
        if (pbgt->GetDrawPriority() >= 0)
            drawListBG.emplace(pbgt->GetDrawPriority(), pbgt);
        return pbgt;
    }

    void CTaskManager::Execute(double elapsedTime)
    {
        // �^�X�NExecute
        auto taskExecute = [this, elapsedTime](TaskList::iterator i, TaskList::iterator ied){
            deque<TaskList::iterator> deleteList;
            deque<TaskList::iterator>::iterator idl, idl_ed;

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
                    if (*i == NULL)OutputLog("catch while execute1 : NULL", SYSLOG_ERROR);
                    else OutputLog("catch while execute1 : %X , %s", *i, typeid(**i).name());
                    break;
                }
#endif
    }
            //�^�X�N��false��Ԃ������̂�����
            if (deleteList.size() != 0){
                idl = deleteList.begin();
                idl_ed = deleteList.end();
                for (; idl != idl_ed; ++idl){
                    i = *idl;
                    (*i)->Terminate();
                    tasks.erase(i);
                }
            }
        };

#ifdef ARRAYBOUNDARY_DEBUG
        if(!AfxCheckMemory()){
            OutputLog("AfxCheckMemory() failed");
            return;
        }
#endif

        //�r���^�X�N�Atop�̂�Execute
        assert(ex_stack.size() != 0);
        shared_ptr<CExclusiveTaskBase> exTsk = ex_stack.top().value;

        if (exTsk)
        {
            bool ex_ret = true;
#ifdef _CATCH_WHILE_EXEC
            try{
#endif
                ex_ret = exTsk->Execute(elapsedTime);
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
                        assert(ex_stack.size() != 0);
                        exTsk = ex_stack.top().value;
                        if (exTsk)
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
        assert(!ex_stack.empty());
        taskExecute(ex_stack.top().SubTaskStartPos, tasks.end());

        //�풓�^�X�NExecute
        taskExecute(bg_tasks.begin(), bg_tasks.end());

        // �V�����^�X�N������ꍇ
        if (exNext){
            //���ݔr���^�X�N��Inactivate
            assert(ex_stack.size() != 0);
            auto& exTsk = ex_stack.top().value;
            if (exTsk && !exTsk->Inactivate(exNext->GetID())){
                //�ʏ�^�X�N��S�Ĕj������
                CleanupPartialSubTasks(ex_stack.top().SubTaskStartPos);

                exTsk->Terminate();
                ex_stack.pop();
            }

            //Add���ꂽ�^�X�N��Initialize���ē˂�����
            const auto it = tasks.emplace(tasks.end(), make_shared<CTaskBase>());				// �_�~�[�^�X�N�}��
            ex_stack.emplace(move(exNext), it);
            ex_stack.top().value->Initialize();

            exNext = nullptr;
        }
    }


    void CTaskManager::Draw()
    {
        TaskList::iterator i, ied;
        shared_ptr<CExclusiveTaskBase> pex;
        auto& drawList = ex_stack.top().drawList;

        //�r���^�X�N���擾
        assert(ex_stack.size() != 0);
        if (ex_stack.top().value && ex_stack.top().value->GetDrawPriority() >= 0){
            pex = ex_stack.top().value;
        }

        auto iv = drawList.begin();
        auto iedv = pex ? drawList.upper_bound(pex->GetDrawPriority()) : drawList.end();
        auto ivBG = drawListBG.begin();
        const auto iedvBG = drawListBG.end();
        auto DrawAndProceed = [&](DrawPriorityMap::iterator& iv)
        {
                    auto is = iv->second.lock();

                    if (is)
                    {
                        is->Draw();
                        ++iv;
                    }
                    else
                        drawList.erase(iv++);
        };
        auto DrawAll = [&]()		// �`��֐�
        {
            while (iv != iedv)
            {
#ifdef _CATCH_WHILE_RENDER
                try{
#endif
                    while (ivBG != iedvBG && ivBG->first <= iv->first)
                        DrawAndProceed(ivBG);
                    DrawAndProceed(iv);
#ifdef _CATCH_WHILE_RENDER
                }catch(...){
                    OutputLog("catch while draw : %X %s", *iv, typeid(*(*iv).lock()).name());
                }
#endif
            }
        };
        //�`��
        DrawAll();

        // �r���^�X�NDraw
        if (pex)
            pex->Draw();

        //�`��
        assert(iv == iedv);
        iedv = drawList.end();
        DrawAll();

        // �����c�����풓�^�X�N����
        while (ivBG != iedvBG)
            DrawAndProceed(ivBG);
    }

    void CTaskManager::RemoveTaskByID(unsigned int id)
    {
        TaskList::iterator i, ied;

        //�ʏ�^�X�N���`�F�b�N
        if (indices.find(id) != indices.end())
        {
            i = tasks.begin();
            ied = tasks.end();
            for (; i != ied; ++i){
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
            for (; i != ied; ++i){
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
        return ex_stack.top().value;
    }

    //�w��ID�̔r���^�X�N�܂�Terminate/pop����
    void CTaskManager::RevertExclusiveTaskByID(unsigned int id)
    {
        bool act = false;
        unsigned int previd = 0;

        assert(ex_stack.size() != 0);
        while (ex_stack.top().value){
            const shared_ptr<CExclusiveTaskBase>& task = ex_stack.top().value;
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
                assert(ex_stack.size() != 0);
            }
        }
    }

    //�ʏ�^�X�N���ꕔ�����j������
    void CTaskManager::CleanupPartialSubTasks(TaskList::iterator it_task)
    {
        TaskList::iterator i, ied;

        i = it_task;
        ied = tasks.end();
        for (; i != ied; ++i){
            shared_ptr<CTaskBase>& delTgt = (*i);
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
        for (; i != ied; ++i){
            OutputLog(typeid(**i).name());
        }

        OutputLog("���풓�^�X�N�ꗗ��");
        //�o�b�N�O���E���h�^�X�N
        i = bg_tasks.begin();
        ied = bg_tasks.end();
        for (; i != ied; ++i){
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
}
