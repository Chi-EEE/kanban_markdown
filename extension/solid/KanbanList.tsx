import styles from './KanbanList.module.css';

import { KanbanMarkdown } from './types';
import TextareaAutosize from "solid-textarea-autosize";

import type { Component } from 'solid-js';
import { Show, For, createSignal, createEffect, onCleanup } from "solid-js";

import { KanbanTask } from './KanbanTask';

type KanbanListProps = {
    kanban_list: KanbanMarkdown.KanbanList;
};

export const KanbanList: Component<KanbanListProps> = (props) => {
    const { kanban_list } = props;

    const [getName, setName] = createSignal<string>(kanban_list.name);
    const [getPreviousName, setPreviousName] = createSignal<string>(kanban_list.name);

    createEffect(() => {
        const currentName = getName();
        const previousName = getPreviousName();
        if (currentName !== previousName) {
            // @ts-ignore
            vscode.postMessage({
                type: 'update',
                path: `list[${previousName}].name`,
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

    return (
        <div class={styles.kanban_list}>
            <TextareaAutosize
                class={styles.kanban_list_title}
                placeholder='Enter list title'
                onBlur={onBlur}
                onKeyPress={onKeyPress}
                textContent={getName()}
            />
            <Show when={kanban_list.tasks}>
                <div class={styles.kanban_task_list}>
                    <For each={kanban_list.tasks}>
                        {(kanban_task, index) => (
                            <KanbanTask kanban_list={kanban_list} kanban_task={kanban_task} />
                        )}
                    </For>
                </div>
            </Show>
        </div>
    );
}
