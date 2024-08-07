import type { Component } from 'solid-js';
import { For, Show, createSignal, createEffect, on } from "solid-js";
import { createStore, produce } from 'solid-js/store';

import {
    closestCenter,
    CollisionDetector,
    createSortable,
    DragDropProvider,
    DragDropSensors,
    DragEventHandler,
    Draggable,
    DragOverlay,
    Droppable,
    SortableProvider
} from '@thisbeyond/solid-dnd';

import Big from 'big.js'
import { arrayMoveImmutable } from 'array-move';

import styles from './App.module.css';
import { KanbanMarkdown } from '../../types';
import { pSBC } from '../../utils';

import { TitleBar } from './TitleBar';
import { KanbanList } from '../content/KanbanList'
import { TaskModal } from './TaskModal';
import { KanbanTask } from '../content/KanbanTask';

export const ORDER_DELTA = 1000;

type AppProps = {
    kanban_board: KanbanMarkdown.KanbanBoard;
};

const App: Component<AppProps> = (props) => {
    const [state, setState] = createStore<KanbanMarkdown.State>({
        selectedList: undefined,
        selectedTask: undefined,
        kanban_board: props.kanban_board,
    });

    const [getTaskModalState, setTaskModalState] = createSignal<boolean>(false);

    function setStyleColor() {
        document.documentElement.style.setProperty('--background-color', state.kanban_board.properties.color);
        // @ts-ignore
        document.documentElement.style.setProperty('--menu-background-color', pSBC(-0.4, state.kanban_board.properties.color));
    }

    createEffect(on(() => state.kanban_board.properties.color, () => {
        setStyleColor();
    }))

    setStyleColor();

    const updateTask = (task: KanbanMarkdown.KanbanTask) => {
        setState("kanban_board", "lists", (list) => list.name === state.selectedList.name, "tasks", (t) => t.name === state.selectedTask.name, task);
    }

    const closestEntity: CollisionDetector = (draggable, droppables, context) => {
        const closestList = closestCenter(
            draggable,
            droppables.filter((droppable) => droppable.data.type === 'list'),
            context
        );
        if (draggable.data.type === 'list') {
            return closestList
        } else if (closestList) {
            const closestTask = closestCenter(
                draggable,
                droppables.filter((droppable) => droppable.data.type === 'task' && droppable.data.list === closestList.id),
                context
            );
            if (!closestTask) {
                return closestList;
            }
            const changingList = draggable.data.list !== closestList.id;
            if (changingList) {
                const belowLastItem = state.kanban_board.lists.filter((kanban_list) => kanban_list.name === closestList.id)[0].name === closestList.id &&
                    draggable.transformed.center.y > closestTask.transformed.center.y;

                if (belowLastItem) return closestList;
            }
            return closestTask;
        }
    };

    const move = (
        draggable: Draggable,
        droppable: Droppable,
        onlyWhenChangingGroup = true
    ) => {
        if (!draggable || !droppable) return;

        const draggableIsList = draggable.data.type === 'list';
        const droppableIsList = droppable.data.type === 'list';

        const draggableListId = draggableIsList
            ? draggable.id
            : draggable.data.list;

        const droppableListId = droppableIsList
            ? droppable.id
            : droppable.data.list;

        if (
            onlyWhenChangingGroup &&
            (draggableIsList || draggableListId === droppableListId)
        ) {
            return;
        }

        let ids, orders, order;

        if (draggableIsList) {
            ids = state.kanban_board.lists.map((kanban_list) => kanban_list.name);
            orders = state.kanban_board.lists.map((_, i) => i);
        } else {
            const kanban_list = state.kanban_board.lists.filter((kanban_list) => kanban_list.name === droppableListId)[0];
            ids = kanban_list.tasks.map((kanban_task) => kanban_task.name + '-' + kanban_task.counter);
            orders = kanban_list.tasks.map((_, i) => i);
        }
        console.log(`===========================================`);
        console.log(`ids: ${ids}`);
        console.log(`orders: ${orders}`);

        if (!draggableIsList && droppableIsList) {
            order = new Big(orders.at(-1) ?? -ORDER_DELTA).plus(ORDER_DELTA).round();
        } else if (draggableIsList && droppableIsList) {
            const draggableIndex = ids.indexOf(draggable.id);
            const droppableIndex = ids.indexOf(droppable.id);
            if (draggableIndex !== droppableIndex) {
                let orderAfter, orderBefore;
                if (draggableIndex === -1 || draggableIndex > droppableIndex) {
                    orderBefore = new Big(orders[droppableIndex]);
                    orderAfter = new Big(
                        orders[droppableIndex - 1] ?? orderBefore.minus(ORDER_DELTA * 2)
                    );
                } else {
                    orderAfter = new Big(orders[droppableIndex]);
                    orderBefore = new Big(
                        orders[droppableIndex + 1] ?? orderAfter.plus(ORDER_DELTA * 2)
                    );
                }

                if (orderAfter !== undefined && orderBefore !== undefined) {
                    order = orderAfter.plus(orderBefore).div(2.0);
                    const rounded = order.round();
                    if (rounded.gt(orderAfter) && rounded.lt(orderBefore)) {
                        order = rounded;
                    }
                }
            }
        }

        if (order !== undefined) {
            setState("kanban_board", "lists", produce((lists: Array<KanbanMarkdown.KanbanList>) => {
                if (draggableIsList) {
                    const oldIndex = lists.findIndex((list) => list.name === draggableListId);
                    const newIndex = Math.min(Math.max(Number(order.toString()), 0), lists.length - 1);
                    lists = arrayMoveImmutable(lists, oldIndex, newIndex);
                } else {
                    const oldList = lists.filter((list) => list.name === draggableListId)[0];
                    if (draggableListId !== droppableListId) {
                        const task = oldList.tasks.filter((task) => task.name + '-' + task.counter === draggable.id)[0];
                        oldList.tasks.splice(oldList.tasks.indexOf(task), 1);
                        const newList = lists.filter((list) => list.name === droppableListId)[0];
                        const newIndex = Math.min(Math.max(Number(order.toString()), 0), newList.tasks.length - 1);
                        newList.tasks.splice(newIndex, 0, task);
                    } else {
                        const oldIndex = oldList.tasks.findIndex((task) => task.name + '-' + task.counter === draggable.id);
                        const newIndex = Math.min(Math.max(Number(order.toString()), 0), oldList.tasks.length - 1);
                        oldList.tasks = arrayMoveImmutable(oldList.tasks, oldIndex, newIndex);
                    }
                }
            }));
        }
    };

    const onDragOver: DragEventHandler = ({ draggable, droppable }) => {
        move(draggable, droppable);
        setDraggingState(true);
    }
    const onDragEnd: DragEventHandler = ({ draggable, droppable }) => {
        move(draggable, droppable, false);
        setDraggingState(false);
    }

    const listNames = () => state.kanban_board.lists.map((kanban_list) => kanban_list.name);

    const [draggingState, setDraggingState] = createSignal<boolean>(false);

    return (
        <div class={styles.App}>
            <TitleBar state={state} setState={setState} />
            <DragDropProvider
                onDragOver={onDragOver}
                onDragEnd={onDragEnd}
                collisionDetector={closestEntity}>
                <DragDropSensors />
                <div class={styles.kanban_board}>
                    <SortableProvider ids={listNames()}>
                        <For each={state.kanban_board.lists}>
                            {(kanban_list) => {
                                const sortedKanbanIds = () => kanban_list.tasks.map((kanban_task) => kanban_task.name + '-' + kanban_task.counter);

                                return (<KanbanList setState={setState} kanban_list={kanban_list} >
                                    <SortableProvider ids={sortedKanbanIds()}>
                                        <For each={kanban_list.tasks}>
                                            {(kanban_task) => (
                                                <KanbanTask
                                                    setState={setState}
                                                    kanban_list={kanban_list}
                                                    kanban_task={kanban_task}
                                                    draggingState={draggingState}
                                                    setTaskModalState={setTaskModalState}
                                                />
                                            )}
                                        </For>
                                    </SortableProvider>
                                </KanbanList>)
                            }}
                        </For>
                    </SortableProvider>
                </div>
            </DragDropProvider>
            {/* <DragOverlay>
                {(draggable) => {
                    return draggable.data.type === 'list' ? (
                        <GroupOverlay name={entity.name} items={groupItems(entity.id)} />
                    ) : (
                        <ItemOverlay name={entity.name} />
                    );
                }}
            </DragOverlay> */}
            <Show when={getTaskModalState()}>
                <TaskModal
                    state={state}
                    setState={setState}
                    setTaskModalState={setTaskModalState}
                    updateTask={updateTask}
                />
            </Show>
        </div>
    );
}

export default App;
