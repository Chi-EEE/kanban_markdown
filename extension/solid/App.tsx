import type { Component } from 'solid-js';
import { For, Show, createSignal, createEffect } from "solid-js";

import styles from './App.module.css';
import { KanbanMarkdown } from './types';
import { pSBC } from './utils';

import { TitleBar } from './TitleBar';
import { KanbanList } from './KanbanList'

type AppProps = {
    kanban_board: KanbanMarkdown.KanbanBoard;
};

const App: Component<AppProps> = (props) => {
    const { kanban_board } = props;

    const [getKanbanBoard, setKanbanBoard] = createSignal<KanbanMarkdown.KanbanBoard>(kanban_board);

    createEffect(() => {
        const kanban_board = getKanbanBoard();
        document.documentElement.style.setProperty('--background-color', kanban_board.properties.color);
        // @ts-ignore
        document.documentElement.style.setProperty('--menu-background-color', pSBC(-0.4, kanban_board.properties.color));
    })

    document.documentElement.style.setProperty('--background-color', kanban_board.properties.color);
    // @ts-ignore
    document.documentElement.style.setProperty('--menu-background-color', pSBC(-0.4, kanban_board.properties.color));

    return (
        <div class={styles.App}>
            <TitleBar kanban_board={getKanbanBoard()} />
            <div class={styles.kanban_board}>
                <Show when={getKanbanBoard().lists}>
                    <For each={getKanbanBoard().lists}>
                        {(kanban_list, index) => (
                            <KanbanList kanban_list={kanban_list} />
                        )}
                    </For>
                </Show>
            </div>
        </div>
    );
}

export default App;
