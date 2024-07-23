/* @refresh reload */
import './index.css';
import { render, hydrate } from 'solid-js/web';

import App from './layout/App';

const root = document.getElementById('root');

if (import.meta.env.DEV && !(root instanceof HTMLElement)) {
    throw new Error(
        'Root element not found. Did you forget to add it to your index.html? Or maybe the id attribute got misspelled?',
    );
}

let rendered: boolean = false;

window.addEventListener('message', function (event) {
    const kanbanBoard = JSON.parse(event.data.text).json;
    if (event.data.type === 'update') {
        if (rendered) {
            // Broken
            // hydrate(() => <App kanban_board={kanbanBoard} />, root!);
        } else {
            render(() => <App kanban_board={kanbanBoard} />, root!);
        }
        rendered = true;
        // @ts-ignore
        vscode.setState({ kanban_board: kanbanBoard });
    }
});

// @ts-ignore
const state = vscode.getState();
if (state) {
    const kanbanBoard = state.kanban_board;
    if (rendered) {
        // Broken
        // hydrate(() => <App kanban_board={kanbanBoard} />, root!);
    } else {
        render(() => <App kanban_board={kanbanBoard} />, root!);
    }
    rendered = true;
}