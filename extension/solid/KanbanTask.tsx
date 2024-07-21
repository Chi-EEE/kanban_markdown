import { KanbanMarkdown } from './types';

import type { Component } from 'solid-js';
import { createSignal, createEffect } from "solid-js";

type KanbanTaskProps = {
    kanban_list: KanbanMarkdown.KanbanList;
    kanban_task: KanbanMarkdown.KanbanTask;
};

export const KanbanTask: Component<KanbanTaskProps> = (props) => {
    const { kanban_list, kanban_task } = props;
    const [getName, setName] = createSignal<string>(kanban_task.name);
    const [getPreviousName, setPreviousName] = createSignal<string>(kanban_task.name);

    createEffect(() => {
        const currentName = getName();
        const previousName = getPreviousName();
        if (currentName !== previousName) {
            // @ts-ignore
            vscode.postMessage({
                type: 'update',
                path: `list[${kanban_list.name}].tasks[${previousName}].name`,
                value: currentName
            });
            setPreviousName(currentName);
        }
    });

    function onBlur(event: FocusEvent) {
        const target = event.target as HTMLTextAreaElement;
        setName(target.value);
    }

    function onKeyPress(event: KeyboardEvent) {
        if (event.key === 'Enter') {
            const target = event.target as HTMLTextAreaElement;
            target.blur();
        }
    }

    function onClick(event: MouseEvent) {
        const target = event.target as HTMLTextAreaElement;
    }

    return (
        <a
            class="block rounded-md p-2 mb-2 white relative cursor-pointer"
            style="background-color: rgba(255, 255, 255, 0.2); width: calc(250px - 20px); box-shadow: 0 1px 0 rgba(0, 0, 0, 0.1);"
            onClick={onClick}
        >
            <span
                class="pointer-events-none w-full bg-transparent border-none outline-none text-white inline-block overflow-hidden break-words whitespace-normal"
                style="background-color: rgba(255, 255, 255, 0.1);"
                onBlur={onBlur}
                onKeyPress={onKeyPress}
            >
                {getName()}
            </span>
        </a>
    );
}
