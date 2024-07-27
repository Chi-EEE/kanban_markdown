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
    const [getSelectedList, setSelectedList] = createSignal<KanbanMarkdown.KanbanList | undefined>(undefined);
    const [getSelectedTask, setSelectedTask] = createSignal<KanbanMarkdown.KanbanTask | undefined>(undefined);

    const [kanban_board, setKanbanBoard] = createStore<KanbanMarkdown.KanbanBoard>(props.kanban_board);
    const [getTaskModalState, setTaskModalState] = createSignal<boolean>(false);

    function setStyleColor() {
        document.documentElement.style.setProperty('--background-color', kanban_board.properties.color);
        // @ts-ignore
        document.documentElement.style.setProperty('--menu-background-color', pSBC(-0.4, kanban_board.properties.color));
    }

    createEffect(on(() => kanban_board.properties.color, () => {
        setStyleColor();
    }))

    setStyleColor();

    return (
        <div class={styles.App}>
            <TitleBar kanban_board={kanban_board} setKanbanBoard={setKanbanBoard} />
            <div class={styles.kanban_board}>
                <Show when={kanban_board.lists}>
                    <For each={kanban_board.lists}>
                        {(kanban_list) => {
                            const kanban_list_props = {
                                kanban_board,
                                setKanbanBoard,
                                kanban_list,
                                setTaskModalState,
                                setSelectedList,
                                setSelectedTask
                            }
                            return <KanbanList {...kanban_list_props} />
                        }}
                    </For>
                </Show>
            </div>
            <Show when={getTaskModalState()}>
                <TaskModal
                    kanban_board={kanban_board}
                    setKanbanBoard={setKanbanBoard}
                    setTaskModalState={setTaskModalState}
                    getSelectedList={getSelectedList}
                    setSelectedTask={setSelectedTask}
                    getSelectedTask={getSelectedTask}
                />
            </Show>
        </div>
    );
}

export default App;
