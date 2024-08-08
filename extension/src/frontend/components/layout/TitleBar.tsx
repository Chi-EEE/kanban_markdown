import styles from './TitleBar.module.css';

import { createSignal, Show, Switch, Match } from "solid-js";
import type { Accessor, Component, Setter } from 'solid-js';
import { KanbanMarkdown } from "../../types";
import { SetStoreFunction } from 'solid-js/store';

type TitleBarProps = {
    state: KanbanMarkdown.State;
    setState: SetStoreFunction<KanbanMarkdown.State>;
};
export const TitleBar: Component<TitleBarProps> = (props) => {
    const { state, setState } = props;

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
                        <h1 class={styles.kanban_board_name}>{state.kanban_board.name}</h1>
                    </Match>
                    <Match when={getTitleState() === "input"}>
                        <input ref={title_input_reference} type="text" class={styles.edit_kanban_board_name_input}
                            onInput={(event) => {
                                const kanban_board_name = (event.target as HTMLInputElement).value;
                                setState("kanban_board", "name", kanban_board_name);
                                // @ts-ignore
                                vscode.postMessage({
                                    commands: [
                                        {
                                            action: 'update',
                                            path: 'name',
                                            value: kanban_board_name
                                        }
                                    ]
                                });
                            }}
                            value={state.kanban_board.name}
                        />
                    </Match>
                </Switch>
            </div>
            <input type="color" class={styles.background_color_picker} value={state.kanban_board.properties.color}
                onInput={(event) => {
                    const color = (event.target as HTMLInputElement).value;
                    setState("kanban_board", "properties", "color", color);
                    // @ts-ignore
                    vscode.postMessage({
                        commands: [
                            {
                                action: 'update',
                                path: 'color',
                                value: color
                            }
                        ]
                    });
                }} />
        </div>
    );
}
