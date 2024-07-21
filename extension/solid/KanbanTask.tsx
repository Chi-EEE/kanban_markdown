import type { Component } from 'solid-js';
import { createSignal, createEffect, onMount } from "solid-js";

import styles from './KanbanTask.module.css';

import { KanbanMarkdown } from './types';

import { applyAutoResize } from './utils';

type KanbanTaskProps = {
    kanban_list: KanbanMarkdown.KanbanList;
    kanban_task: KanbanMarkdown.KanbanTask;
};

export const KanbanTask: Component<KanbanTaskProps> = (props) => {
    const { kanban_list, kanban_task } = props;

    const [getName, setName] = createSignal<string>(kanban_task.name);
    const [getPreviousName, setPreviousName] = createSignal<string>(kanban_task.name);

    let kanban_task_name_reference: HTMLAnchorElement;

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
        target.value = target.value.replace(/[\v\n]+/g, '').trim();
        setName(target.value);
    }

    function onKeyPress(event: KeyboardEvent) {
        const target = event.target as HTMLTextAreaElement;
        if (event.key === 'Enter' && !event.shiftKey) {
            target.blur();
        } else {
            applyAutoResize(target);
        }
    }

    function onClick(event: MouseEvent) {
        const target = event.target as HTMLTextAreaElement;
    }

    onMount(() => {
        if (kanban_task_name_reference) {
            applyAutoResize(kanban_task_name_reference);
        }
    });

    return (
        <a class={styles.kanban_task} ref={kanban_task_name_reference} onClick={onClick} >
            <span class={styles.kanban_task_title} onBlur={onBlur} onKeyPress={onKeyPress}>
                {getName()}
            </span>
        </a>
    );
}
