import { KanbanMarkdown } from './types';

import type { Component } from 'solid-js';
import { For, Show, createSignal } from "solid-js";

import { TitleBar } from './TitleBar';
import { KanbanList } from './KanbanList'

type AppProps = {
    kanban_board: KanbanMarkdown.KanbanBoard;
};

const App: Component<AppProps> = (props) => {
    const { kanban_board } = props;

    const [getKanbanBoard, setKanbanBoard] = createSignal<KanbanMarkdown.KanbanBoard>(kanban_board);

    return (
        <div style="height:100vh;">
            <TitleBar kanban_board={getKanbanBoard()} />
            <Show when={getKanbanBoard().lists}>
                <div
                    class="flex items-start mt-16 p-5 w-full overflow-x-auto whitespace-nowrap"
                    style="height: calc(100%-60px);"
                >
                    <For each={getKanbanBoard().lists}>
                        {(list, index) => (
                            <KanbanList kanban_list={list} />
                        )}
                    </For>
                </div>
            </Show>
        </div>
    );
}

export default App;
