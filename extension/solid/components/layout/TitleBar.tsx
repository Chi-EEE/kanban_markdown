import styles from './TitleBar.module.css';

import { createSignal } from "solid-js";
import type { Component } from 'solid-js';
import { KanbanMarkdown } from "../../types";

type TitleBarProps = {
    kanban_board: KanbanMarkdown.KanbanBoard;
    setColor: (color: string) => void;
};
export const TitleBar: Component<TitleBarProps> = (props) => {
    const { kanban_board, setColor } = props;

    const [getName, setName] = createSignal<string>(kanban_board.name);

    return (
        <div class={styles.title_bar}>
            <div>
                <h1 class={styles.kanban_board_name}>{getName()}</h1>
                <input type="text" class={styles.edit_kanban_board_name_input} />
            </div>
            <input type="color" class={styles.background_color_picker} value={kanban_board.properties.color} 
            onInput={(event) => {
                const color = (event.target as HTMLInputElement).value;
                setColor(color)
                // @ts-ignore
                vscode.postMessage({
                    type: 'update',
                    path: 'color',
                    value: color
                });
            }}/>
        </div>
    );
}
