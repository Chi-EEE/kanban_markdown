import type { Component } from 'solid-js';
import { For, Show, createSignal, createEffect } from "solid-js";

import styles from './App.module.css';
import { KanbanMarkdown } from '../../types';
import { pSBC } from '../../utils';

import { TitleBar } from './TitleBar';
import { KanbanList } from '../content/KanbanList'
import { TaskModal } from './TaskModal';

type AppProps = {
    kanban_board: KanbanMarkdown.KanbanBoard;
};

const App: Component<AppProps> = (props) => {
    const { kanban_board } = props;

    const [getKanbanBoard, setKanbanBoard] = createSignal<KanbanMarkdown.KanbanBoard>(kanban_board);
    const [getTaskModalState, setTaskModalState] = createSignal<boolean>(false);

    const [getColor, setColor] = createSignal<string>(kanban_board.properties.color);

    createEffect(() => {
        document.documentElement.style.setProperty('--background-color', getColor());
        // @ts-ignore
        document.documentElement.style.setProperty('--menu-background-color', pSBC(-0.4, getColor()));
    })

    document.documentElement.style.setProperty('--background-color', getColor());
    // @ts-ignore
    document.documentElement.style.setProperty('--menu-background-color', pSBC(-0.4, getColor()));

    return (
        <div class={styles.App}>
            <TitleBar kanban_board={getKanbanBoard()} setColor={setColor} />
            <div class={styles.kanban_board}>
                <Show when={getKanbanBoard().lists}>
                    <For each={getKanbanBoard().lists}>
                        {(kanban_list, index) => (
                            <KanbanList kanban_list={kanban_list} setTaskModalState={setTaskModalState} />
                        )}
                    </For>
                </Show>
            </div>
            <Show when={getTaskModalState()}>
                <TaskModal kanban_board={getKanbanBoard()} setTaskModalState={setTaskModalState} />
            </Show>
        </div>
    );
}

export default App;
