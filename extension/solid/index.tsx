/* @refresh reload */
import './index.css';
import { render, hydrate } from 'solid-js/web';

import App from './App';

const root = document.getElementById('root');

if (import.meta.env.DEV && !(root instanceof HTMLElement)) {
    throw new Error(
        'Root element not found. Did you forget to add it to your index.html? Or maybe the id attribute got misspelled?',
    );
}

let rendered: boolean = false;

window.addEventListener('message', function (event) {
    const { type, text: { json: kanbanBoard } } = event.data;
    if (type === 'update') {
        if (rendered) {
            hydrate(() => <App kanban_board={kanbanBoard} />, root!);
        } else {
            render(() => <App kanban_board={kanbanBoard} />, root!);
        }
        rendered = true;
        // @ts-ignore
        vscode.setState({ json: kanbanBoard });
    }
});

const state = vscode.getState();
if (state) {
    const { json: kanbanBoard } = state;
    if (rendered) {
        hydrate(() => <App kanban_board={kanbanBoard} />, root!);
    } else {
        render(() => <App kanban_board={kanbanBoard} />, root!);
    }
    rendered = true;
}