import type { Accessor, Component } from 'solid-js';
import { For, Show, createSignal, createEffect, on, onMount } from "solid-js";
import { createStore, produce } from 'solid-js/store';

import {
    closestCenter,
    CollisionDetector,
    DragDropProvider,
    DragDropSensors,
    DragEventHandler,
    Draggable,
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
import { TemporaryKanbanList } from '../content/TemporaryKanbanList';

type AppProps = {
    kanban_board: KanbanMarkdown.KanbanBoard;
};

const [state, setState] = createStore<KanbanMarkdown.State>({
    selectedList: undefined,
    selectedTask: undefined,
    kanban_board: undefined,
    window_state: {
        taskModalState: false,
        scrollTop: 0,
        scrollLeft: 0
    }
});

const App: Component<AppProps> = (props) => {
    const list_name_tracker_map = new Map<string, KanbanMarkdown.DuplicateNameTracker>();
    for (let entry of Object.entries(props.kanban_board.list_name_tracker_map)) {
        let key = entry[0];
        let value: KanbanMarkdown.DuplicateNameTracker = entry[1];
        list_name_tracker_map.set(key, new KanbanMarkdown.DuplicateNameTracker(value.counter, new Set<number>(value.used_hash)));
    }
    props.kanban_board.list_name_tracker_map = list_name_tracker_map;

    const task_name_tracker_map = new Map<string, KanbanMarkdown.DuplicateNameTracker>();
    for (let entry of Object.entries(props.kanban_board.task_name_tracker_map)) {
        let key = entry[0];
        let value: KanbanMarkdown.DuplicateNameTracker = entry[1];
        task_name_tracker_map.set(key, new KanbanMarkdown.DuplicateNameTracker(value.counter, new Set<number>(value.used_hash)));
    }
    props.kanban_board.task_name_tracker_map = task_name_tracker_map;

    setState(produce((state) => {
        state.kanban_board = props.kanban_board;

        if (state.selectedList) {
            const kanban_list = state.kanban_board.lists.find((list) => list.name === state.selectedList.name && list.counter === state.selectedList.counter);
            state.selectedList = kanban_list;
            if (state.selectedTask) {
                state.selectedTask = kanban_list.tasks
                    .find((task) => task.name === state.selectedTask.name && task.counter === state.selectedTask.counter);
            }
        }
    }));


    function setStyleColor(color: string) {
        document.documentElement.style.setProperty('--background-color', color);
        // @ts-ignore
        document.documentElement.style.setProperty('--menu-background-color', pSBC(-0.4, color));
    }

    createEffect(on(() => state.kanban_board.properties.color, (color) => {
        setStyleColor(color);
    }))

    setStyleColor(state.kanban_board.properties.color);

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

    interface DraggingTask {
        type: 'task';
        data: {
            index: Accessor<number>;
            name: string;
            counter: number;
            list: string;
        };
    };

    interface DraggingList {
        type: 'list';
        data: {
            index: Accessor<number>;
            name: string;
            counter: number;
        };
    };

    let draggableItem: DraggingTask | DraggingList | undefined = undefined;
    let value = {
        index: 0,
        destination: undefined
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
            map = new Map(state.kanban_board.lists.map((kanban_list, i) => [`list-${kanban_list.name}-${kanban_list.counter}`, i]));
        } else {
            const kanban_list = state.kanban_board.lists.find((kanban_list) => `list-${kanban_list.name}-${kanban_list.counter}` === droppableListId);
            if (!kanban_list) {
                return;
            }
            map = new Map(kanban_list.tasks.map((kanban_task, i) => [`task-${kanban_task.name}-${kanban_task.counter}`, i]));
        }

        setState("kanban_board", "lists", produce((lists: Array<KanbanMarkdown.KanbanList>) => {
            if (draggableIsList) {
                const oldIndex = lists.findIndex((list) => `list-${list.name}-${list.counter}` === draggableListId);
                const newIndex = lists.findIndex((list) => `list-${list.name}-${list.counter}` === droppableListId);
                arrayMoveMutable(lists, oldIndex, newIndex);
                value.index = newIndex;
            } else {
                const list = lists.find((list) => `list-${list.name}-${list.counter}` === draggableListId);
                if (!list) return;
                if (draggableListId !== droppableListId) {
                    const task = list.tasks.find((task) => `task-${task.name}-${task.counter}` === draggable.id);
                    if (!task) return;
                    list.tasks.splice(list.tasks.indexOf(task), 1);
                    const newList = lists.find((list) => `list-${list.name}-${list.counter}` === droppableListId);
                    if (!newList) return;
                    const newIndex = map.get(droppable.id as string) || map.size;
                    newList.tasks.splice(newIndex, 0, task);
                    value.index = newIndex;
                    value.destination = `list["${encodeURI(newList.name)}"][${newList.counter}].tasks`;
                } else {
                    const oldIndex = list.tasks.findIndex((task) => `task-${task.name}-${task.counter}` === draggable.id);
                    const newIndex = map.get(droppable.id as string);
                    arrayMoveMutable(list.tasks, oldIndex, newIndex);
                    value.index = newIndex;
                }
            }
        }));
    };

    const onDragStart: DragEventHandler = ({ draggable, droppable }) => {
        if (!draggable) {
            return;
        }
        switch (draggable.data.type) {
            case 'list':
                draggableItem = {
                    type: 'list',
                    // @ts-ignore
                    data: draggable.data,
                };
                break;
            case 'task':
                draggableItem = {
                    type: 'task',
                    // @ts-ignore
                    data: draggable.data,
                };
                break;
        };
        setDraggingState(true);
    }

    let lastDragOver = 0;
    const onDragOver: DragEventHandler = ({ draggable, droppable }) => {
        if (Date.now() - lastDragOver < 5) return;
        lastDragOver = Date.now();
        move(draggable, droppable);
    }

    const onDragEnd: DragEventHandler = ({ draggable, droppable }) => {
        move(draggable, droppable, false);
        let command = undefined;
        switch (draggableItem.type) {
            case 'list':
                const draggableList = draggableItem as DraggingList;
                if (draggableList.data.index() !== value.index) {
                    command = {
                        action: 'move',
                        path: `list["${draggableList.data.name}"][${draggableList.data.counter}]`,
                        value: value,
                    }
                }
                break;
            case 'task':
                const draggableTask = draggableItem as DraggingTask;
                const list = state.kanban_board.lists.find((list) => `list-${list.name}-${list.counter}` === draggableTask.data.list);
                const path = `list["${list.name}"][${list.counter}].tasks["${draggableTask.data.name}"][${draggableTask.data.counter}]`;
                if (!value.destination && draggableTask.data.index() !== value.index || value.destination && !path.includes(value.destination)) {
                    command = {
                        action: 'move',
                        path: path,
                        value: value,
                    };
                }
                break;
        }
        if (command) {
            // @ts-ignore
            vscode.postMessage({
                commands: [command]
            });
        }
        draggableItem = undefined;
        value = {
            index: 0,
            destination: undefined
        }
        setDraggingState(false);
    }

    const listNames = () => state.kanban_board.lists.map((kanban_list) => `list-${kanban_list.name}-${kanban_list.counter}`);

    const [draggingState, setDraggingState] = createSignal<boolean>(false);

    const [getAddButtonVisiblity, setAddButtonVisiblity] = createSignal<boolean>(true);
    const [getListTextAreaReference, setListTextAreaReference] = createSignal<HTMLTextAreaElement>();

    let kanbanBoardRef: HTMLDivElement;

    onMount(() => {
        kanbanBoardRef.scrollTop = state.window_state.scrollTop;
        kanbanBoardRef.scrollLeft = state.window_state.scrollLeft;
    });

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
                <div class={styles.kanban_board}
                    ref={kanbanBoardRef}
                    onScroll={(event) => {
                        setState("window_state", produce((window_state) => {
                            window_state.scrollTop = kanbanBoardRef.scrollTop;
                            window_state.scrollLeft = kanbanBoardRef.scrollLeft;
                        }));
                    }}>
                    <SortableProvider ids={listNames()}>
                        <For each={state.kanban_board.lists}>
                            {(kanban_list, list_index) => {
                                const sortedKanbanIds = () => kanban_list.tasks.map((kanban_task) => `task-${kanban_task.name}-${kanban_task.counter}`);

                                return (<KanbanList
                                    list_index={list_index}
                                    state={state}
                                    setState={setState}
                                    kanban_list={kanban_list}
                                >
                                    <SortableProvider ids={sortedKanbanIds()}>
                                        <For each={kanban_list.tasks}>
                                            {(kanban_task, task_index) => (
                                                <KanbanTask
                                                    task_index={task_index}
                                                    state={state}
                                                    setState={setState}
                                                    kanban_list={kanban_list}
                                                    kanban_task={kanban_task}
                                                    draggingState={draggingState}
                                                />
                                            )}
                                        </For>
                                    </SortableProvider>
                                </KanbanList>)
                            }}
                        </For>
                    </SortableProvider>
                    <Show when={getAddButtonVisiblity()} fallback={
                        <TemporaryKanbanList
                            state={state}
                            setState={setState}
                            setAddButtonVisiblity={setAddButtonVisiblity}
                            setListTextAreaReference={setListTextAreaReference}
                        />
                    }>
                        <button id={styles.add_list_button} onClick={() => {
                            setAddButtonVisiblity(false);
                            getListTextAreaReference().focus();
                        }}>Add another list +</button>
                    </Show>
                </div>
            </DragDropProvider>
            <Show when={state.window_state.taskModalState}>
                <TaskModal
                    state={state}
                    setState={setState}
                />
            </Show>
        </div>
    );
}

export default App;
