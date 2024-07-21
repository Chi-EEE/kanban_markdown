
import { createSignal } from "solid-js";
import type { Component } from 'solid-js';
import { KanbanMarkdown } from "./types";

type TitleBarProps = {
    kanban_board: KanbanMarkdown.KanbanBoard;
};
export const TitleBar: Component<TitleBarProps> = (props) => {
    const { kanban_board } = props;

    const [getName, setName] = createSignal<string>(kanban_board.name);

    return (
        <div class="text-white w-full p-3 box-border fixed top-0 z-50 flex justify-between items-center" style="background-color: rgba(0, 0, 0, 0.4);">
            <div id="editable-title">
                <h1 class="m-0 pl-5">{getName()}</h1>
                <input type="text" id="edit-title-input" />
            </div>
            <input type="color" id="background-color-picker" value="#A0A0A0" />
        </div>
    );
}
