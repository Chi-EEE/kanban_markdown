import styles from './TitleBar.module.css';

import { createSignal, Show, Switch, Match } from "solid-js";
import type { Component, Setter } from 'solid-js';
import { KanbanMarkdown } from "../../types";

type TitleBarProps = {
    kanban_board: KanbanMarkdown.KanbanBoard;
    setColor: Setter<string>;
};
export const TitleBar: Component<TitleBarProps> = (props) => {
    const { kanban_board, setColor } = props;

    const [getName, setName] = createSignal<string>(kanban_board.name);
    const [getTitleState, setTitleState] = createSignal<string>("text");

    let title_input_reference: HTMLInputElement;

    return (
        <div class={styles.title_bar}>
            <div
                onClick={() => {
                    setTitleState("input");
                    title_input_reference.focus();
                }}
                onMouseLeave={() => setTitleState("text")}
            >
                <Switch>
                    <Match when={getTitleState() == "text"}>
                        <h1 class={styles.kanban_board_name}>{getName()}</h1>
                    </Match>
                    <Match when={getTitleState() === "input"}>
                        <input ref={title_input_reference} type="text" class={styles.edit_kanban_board_name_input}
                            onInput={(event) => {
                                setName((event.target as HTMLInputElement).value);
                            }}
                            value={getName()}
                        />
                    </Match>
                </Switch>
            </div>
            <input type="color" class={styles.background_color_picker} value={kanban_board.properties.color}
                onInput={(event) => {
                    const color = (event.target as HTMLInputElement).value;
                    setColor(color)
                    // @ts-ignore
                    vscode.postMessage({
                        commands: [
                            {
                                type: 'update',
                                path: 'color',
                                value: color
                            }
                        ]
                    });
                }} />
        </div>
    );
}
