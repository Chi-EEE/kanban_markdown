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

import { arrayMoveMutable } from 'array-move';

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
            return closestList;
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
                const belowLastItem = draggable.transformed.center.y > closestTask.transformed.center.y;

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
        if (!draggable || !droppable) {
            return;
        }

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


        let map: Map<string, number>;

        if (draggableIsList) {
            map = new Map(state.kanban_board.lists.map((kanban_list, i) => [kanban_list.name, i]));
        } else {
            const kanban_list = state.kanban_board.lists.find((kanban_list) => kanban_list.name === droppableListId);
            if (!kanban_list) {
                return;
            }
            map = new Map(kanban_list.tasks.map((kanban_task, i) => [`${kanban_task.name}-${kanban_task.counter}`, i]));
        }

        setState("kanban_board", "lists", produce((lists: Array<KanbanMarkdown.KanbanList>) => {
            if (draggableIsList) {
                const oldIndex = lists.findIndex((list) => list.name === draggableListId);
                const newIndex = lists.findIndex((list) => list.name === droppableListId);
                arrayMoveMutable(lists, oldIndex, newIndex);
            } else {
                const oldList = lists.find((list) => list.name === draggableListId);
                if (!oldList) return;
                if (draggableListId !== droppableListId) {
                    const task = oldList.tasks.find((task) => task.name + '-' + task.counter === draggable.id);
                    if (!task) return;
                    oldList.tasks.splice(oldList.tasks.indexOf(task), 1);
                    const newList = lists.find((list) => list.name === droppableListId);
                    if (!newList) return;
                    const newIndex = map.get(draggable.id as string);
                    newList.tasks.splice(newIndex, 0, task);
                } else {
                    const oldIndex = oldList.tasks.findIndex((task) => task.name + '-' + task.counter === draggable.id);
                    const newIndex = map.get(droppable.id as string);
                    arrayMoveMutable(oldList.tasks, oldIndex, newIndex);
                }
            }
        }));
    };

    const onDragStart: DragEventHandler = ({ draggable, droppable }) => {
        setDraggingState(true);
    }

    let lastDragOver = 0;
    const onDragOver: DragEventHandler = ({ draggable, droppable }) => {
        if (Date.now() - lastDragOver < 10) return;
        lastDragOver = Date.now();
        move(draggable, droppable);
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
                onDragStart={onDragStart}
                onDragOver={onDragOver}
                onDragEnd={onDragEnd}
                collisionDetector={closestEntity}
            >
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
