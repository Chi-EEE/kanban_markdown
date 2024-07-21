import type { Component } from 'solid-js';
import { Show, For, createSignal, createEffect, onMount } from "solid-js";

import styles from './KanbanList.module.css';

import { KanbanMarkdown } from './types';
import { KanbanTask } from './KanbanTask';

import { applyAutoResize } from './utils';

type KanbanListProps = {
    kanban_list: KanbanMarkdown.KanbanList;
};

export const KanbanList: Component<KanbanListProps> = (props) => {
    const { kanban_list } = props;

    const [getName, setName] = createSignal<string>(kanban_list.name);
    const [getPreviousName, setPreviousName] = createSignal<string>(kanban_list.name);

    let kanban_list_name_reference: HTMLTextAreaElement | undefined;

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
        const target = event.target as HTMLTextAreaElement
        target.value = target.value.replace(/[\v\n]+/g, '').trim();
        setName(target.value);
        applyAutoResize(target);
    }

    function onKeyPress(event: KeyboardEvent) {
        const target = event.target as HTMLTextAreaElement;
        if (event.key === 'Enter' && !event.shiftKey) {
            target.blur();
        } else {
            applyAutoResize(target);
        }
    }

    onMount(() => {
        if (kanban_list_name_reference) {
            applyAutoResize(kanban_list_name_reference);
        }
    });

    return (
        <div class={styles.kanban_list}>
            <textarea
                ref={kanban_list_name_reference}
                class={styles.kanban_list_name}
                placeholder='Enter list name'
                onBlur={onBlur}
                onKeyPress={onKeyPress}
            >
                {getName()}
            </textarea>
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
