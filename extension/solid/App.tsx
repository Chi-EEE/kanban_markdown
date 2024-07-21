import { KanbanMarkdown } from './types';

import type { Component } from 'solid-js';
import { For, Show, createSignal, createEffect, onCleanup } from "solid-js";

import { TitleBar } from './TitleBar';
import { KanbanList } from './KanbanList'

const App: Component = () => {
    const default_kanban_board = {
        name: '',
        properties: {
            color: '',
            created: 0,
            last_modified: 0,
            version: 0,
            checksum: ''
        },
        description: '',
        labels: [],
        lists: []
    }

    const [getKanbanBoard, setKanbanBoard] = createSignal<KanbanMarkdown.KanbanBoard>(default_kanban_board);

    function loadKanbanBoard(data: KanbanMarkdown.KanbanBoard) {
        setKanbanBoard(data);
    }

    function handleMessage(event: MessageEvent) {
        const { type, text: { json: kanbanBoard } } = event.data;
        if (type === 'update') {
            loadKanbanBoard(kanbanBoard);
            // @ts-ignore
            vscode.setState({ json: kanbanBoard });
        }
    }

    window.addEventListener('message', handleMessage);

    onCleanup(() => {
        window.removeEventListener('message', handleMessage);
    });

    return (
        <div>
            <TitleBar kanban_board={getKanbanBoard()} />
            <Show when={getKanbanBoard().lists}>
                <div
                    class="flex items-start mt-16 p-5 w-full overflow-x-auto whitespace-nowrap"
                    style="height: calc(100%-60px)"
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
