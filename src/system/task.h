﻿/*!
*	@file
*	@brief タスク(?)管理・定義
*/
#pragma once
#include <vector>
#include <deque>
#include <string>
#include <map>
#include <list>
#include <unordered_map>
#include <memory>
#include <functional>
#include <type_traits>
#include <utility>

#ifdef __clang__
#   if !__has_feature(cxx_noexcept)
#       define NOEXCEPT
#   else
#       define NOEXCEPT noexcept
#   endif
#elif (defined(_MSC_FULL_VER) && _MSC_FULL_VER < 190023026) || (defined(__GNUC__) && (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 6) || !defined(__GXX_EXPERIMENTAL_CXX0X__)))
#define NOEXCEPT
#else
#define NOEXCEPT noexcept
#endif

/*!
*	@defgroup Tasks
*	@brief タスク
*
*	CTaskBaseを継承したクラスは、メインループから呼ばれる更新・描画処理関数を持っています。
*	システムはこのクラスのリストを持っています。
*	タイトル・キャラセレ・試合 などのゲームの状態の変更は、
*	これらタスククラスの切り替えによって行われます。
*/

namespace GTF
{
    using namespace std;

    /*!
    *	@ingroup Tasks
    *	@brief	基本タスク
    *
    *	・Executeでfalseを返すと破棄される
    *	・親の排他タスクが変更されたとき、破棄される
    */
    class CTaskBase
    {
    public:
        virtual ~CTaskBase(){}
        virtual void Initialize(){}							//!< ExecuteまたはDrawがコールされる前に1度だけコールされる
        virtual bool Execute(double /* elapsedTime */)
                            {return(true);}					//!< 毎フレームコールされる
        virtual void Terminate(){}							//!< タスクのリストから外されるときにコールされる（その直後、deleteされる）
        virtual void Draw(){}								//!< 描画時にコールされる
        virtual unsigned int GetID() const { return 0; }	//!< 0以外を返すようにした場合、マネージャに同じIDを持つタスクがAddされたとき破棄される
        virtual int GetDrawPriority() const { return -1; }	//!< 描画プライオリティ。低いほど後順に（手前に）Draw処理。マイナスならば表示しない
    };


    /*!
    *	@ingroup Tasks
    *	@brief 排他タスク? =ゲームのシーンと考えてください。
    *
    *	・他の排他タスクと一緒には動作(Execute)しない
    *	・他の排他タスクが追加された場合、Inactivateがコールされ、そこでfalseを返すと
    *		破棄される。trueを返すとExecute、WndMessageがコールされない状態になり、
    *		新規の排他タスクが全て破棄されたときにActivateが呼ばれ、処理が再開する。
    *	・通常タスクとの親子関係を持つ。
    *	・AddTask実行後、一度Executeが実行されるまで追加が保留される。その後に追加された通常タスクは子タスクとなる。
    */
    class CExclusiveTaskBase : public CTaskBase
    {
    public:
        CExclusiveTaskBase(bool fallthroughDraw = false) NOEXCEPT : isFallthroughDraw(fallthroughDraw) {}
        virtual ~CExclusiveTaskBase(){}
        virtual void Activate(unsigned int /* prvTaskID */){}				//!< Executeが再開されるときに呼ばれる
        virtual bool Inactivate(unsigned int /* nextTaskID */){return true;}//!< 他の排他タスクが開始したときに呼ばれる

        virtual int GetDrawPriority() const override {return 0;}				//!< 描画プライオリティ取得メソッド
        bool IsFallthroughDraw() const NOEXCEPT { return isFallthroughDraw; }		//!< 一つ下の階層のタスクのDrawを実行するかどうか

    private:
        const bool isFallthroughDraw = false;
    };



    /*!
    *	@ingroup Tasks
    *	@brief 常駐タスク
    *
    *	・基本タスクと違い、排他タスクが変更されても破棄されない
    *	・Enabledでないときには Execute , WndMessage をコールしない
    */
    class CBackgroundTaskBase : public CTaskBase
    {
    public:
        virtual ~CBackgroundTaskBase(){}

        bool IsEnabled() const NOEXCEPT { return m_isEnabled; }
        void Enable() NOEXCEPT { m_isEnabled = true; }
        void Disable() NOEXCEPT { m_isEnabled = false; }

    private:
        bool m_isEnabled=true;
    };




    /*!
    *	@ingroup System
    *	@brief タスク管理クラス
    *
    *	タスク継承クラスのリストを管理し、描画、更新を行う。
    *
    *	実行中に例外が起こったとき、どのクラスが例外を起こしたのかをログに吐き出す。
    *	その際に実行時型情報からクラス名を取得しているので、コンパイルの際には
    *	実行時型情報(RTTIと表記される場合もある)をONにすること。
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

        //! 追加したタスクはCTaskManager内部で自動的に破棄されるので、呼び出し側でdeleteしないこと。
        TaskPtr AddTask(CTaskBase *newTask);		        //!< タスク追加
        ExTaskPtr AddTask(CExclusiveTaskBase *newTask);     //!< 排他タスク追加
        BgTaskPtr AddTask(CBackgroundTaskBase *newTask);    //!< 常駐タスク追加
        void RemoveTaskByID(unsigned int id);				//!< 指定IDを持つタスクの除去　※注：Exclusiveタスクはチェックしない
        void RevertExclusiveTaskByID(unsigned int id);		//!< 指定IDの排他タスクまでTerminate/popする

        //! 最上位にあるエクスクルーシブタスクをゲト
        ExTaskPtr GetTopExclusiveTask() const
        {
            return ex_stack.back().value;
        }

        //! タスクの自動生成
        template <class C, typename... A, class PC = shared_ptr<C>,
            typename enable_if<
                integral_constant<bool, is_base_of<CBackgroundTaskBase, C>::value ||
                is_base_of<CExclusiveTaskBase, C>::value
                >::value, std::nullptr_t>::type = nullptr>
            PC AddNewTask(A&&... args)
        {
            return static_pointer_cast<C>(AddTask(new C(forward<A>(args)...)).lock());
        }
        template <class C, typename... A, class PC = shared_ptr<C>,
            typename enable_if<
                integral_constant<bool, !is_base_of<CBackgroundTaskBase, C>::value &&
                !is_base_of<CExclusiveTaskBase, C>::value
                >::value, std::nullptr_t>::type = nullptr>
            PC AddNewTask(A&&... args)
        {
            return static_pointer_cast<C>(AddTaskGuaranteed(new C(forward<A>(args)...)).lock());
        }

        //!指定IDの通常タスク取得
        TaskPtr FindTask(unsigned int id) const
        {
            const auto result = indices.find(id);
            return (result != indices.end()) ? result->second : TaskPtr();
        }

        //!指定IDの常駐タスク取得
        BgTaskPtr FindBGTask(unsigned int id) const
        {
            const auto result = bg_indices.find(id);
            return (result != bg_indices.end()) ? result->second : BgTaskPtr();
        }

        //! 任意のクラス型のタスクを取得（通常・常駐兼用）
        template<class T> shared_ptr<T> FindTask(unsigned int id) const
        {
            return dynamic_pointer_cast<T>(FindTask_impl<T>(id).lock());
        }

        void Execute(double elapsedTime);					//!< 各タスクのExecute関数をコールする
        void Draw();										//!< 各タスクをプライオリティ順にDrawする

        //!< 排他タスクが全部なくなっちゃったかどうか
        bool ExEmpty() const    {
            return ex_stack.empty();
        }

        //デバッグ
        void DebugOutputTaskList();							//!< 現在リストに保持されているクラスのクラス名をデバッグ出力する

    protected:
        using TaskList = list<shared_ptr<CTaskBase>>;
        using BgTaskList = list<shared_ptr<CBackgroundTaskBase>>;
        using DrawPriorityMap = multimap<int, TaskPtr, greater<int>>;

        struct ExTaskInfo {
            const shared_ptr<CExclusiveTaskBase> value;		//!< 排他タスクのポインタ
            const TaskList::iterator SubTaskStartPos;		//!< 依存する通常タスクの開始地点
            DrawPriorityMap drawList;						//!< Draw順ソート用コンテナ。排他タスク自身や、DrawFallthrough時は一つ下の階層のDrawリストも含まれる。

            ExTaskInfo(shared_ptr<CExclusiveTaskBase>& source, TaskList::iterator startPos) NOEXCEPT
                : value(source), SubTaskStartPos(startPos)
            {
            }
            ExTaskInfo(shared_ptr<CExclusiveTaskBase>&& source, TaskList::iterator startPos) NOEXCEPT
                : value(move(source)), SubTaskStartPos(startPos)
            {
            }

        };
        using ExTaskStack = deque<ExTaskInfo>;

        //! 通常タスクを全てTerminate , deleteする
        void CleanupAllSubTasks()    {
            CleanupPartialSubTasks(tasks.begin());
        }

        TaskPtr AddTaskGuaranteed(CTaskBase *newTask);		        //!< タスク追加（エラー検出無し）
        void CleanupPartialSubTasks(TaskList::iterator it_task);	//!< 一部の通常タスクをTerminate , deleteする

        //! ログ出力
        void OutputLog(string /* s */, ...)
        {
            // Not Implemented
        }

    private:
        template<class T,
            typename enable_if<
                integral_constant<bool, !is_base_of<CBackgroundTaskBase, T>::value &&
                !is_base_of<CExclusiveTaskBase, T>::value
                >::value, std::nullptr_t>::type = nullptr>
            TaskPtr FindTask_impl(unsigned int id) const
        {
            return FindTask(id);
        }
        template<class T, typename enable_if<is_base_of<CBackgroundTaskBase, T>::value, std::nullptr_t>::type = nullptr>
            BgTaskPtr FindTask_impl(unsigned int id) const
        {
            return FindBGTask(id);
        }

        //! タスクExecute
        template<class T, typename I = typename T::iterator, class QI = deque<I>, typename I_QI = typename QI::iterator>
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

            //タスクでfalseを返したものを消す
            I_QI idl = deleteList.begin();
            const I_QI idl_ed = deleteList.end();

            for (; idl != idl_ed; ++idl){
                i = *idl;
                (*i)->Terminate();
                tasks.erase(i);
            }
        }

        TaskList tasks;								//!< 現在動作ちゅうのタスクリスト
        BgTaskList bg_tasks;						//!< 常駐タスクリスト
        ExTaskStack ex_stack;						//!< 排他タスクのスタック。topしか実行しない

        shared_ptr<CExclusiveTaskBase> exNext = nullptr;		//!< 現在フレームでAddされた排他タスク
        DrawPriorityMap drawListBG;					//!< Draw順ソート用コンテナ（常駐タスク）
        unordered_map<unsigned int, TaskPtr> indices;
        unordered_map<unsigned int, BgTaskPtr> bg_indices;
    };


}

#ifdef GTF_HEADER_ONLY
#   include "task.cpp"
#endif
