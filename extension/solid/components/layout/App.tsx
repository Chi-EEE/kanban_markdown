import type { Component } from 'solid-js';
import { For, Show, createSignal, createEffect, on } from "solid-js";

import styles from './App.module.css';
import { KanbanMarkdown } from '../../types';
import { pSBC } from '../../utils';

import { TitleBar } from './TitleBar';
import { KanbanList } from '../content/KanbanList'
import { TaskModal } from './TaskModal';
import { createStore } from 'solid-js/store';

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

    return (
        <div class={styles.App}>
            <TitleBar state={state} setState={setState} />
            <div class={styles.kanban_board}>
                <For each={state.kanban_board.lists}>
                    {(kanban_list) => {
                        const kanban_list_props = {
                            setState,
                            kanban_list,
                            setTaskModalState,
                        }
                        return <KanbanList {...kanban_list_props} />
                    }}
                </For>
            </div>
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
