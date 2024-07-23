import type { Component, Setter } from 'solid-js';
import { createSignal, createEffect, onMount, Show, For } from "solid-js";

import styles from './KanbanTask.module.css';

import { KanbanMarkdown } from '../../types';

import { applyAutoResize } from '../../utils';

type KanbanTaskProps = {
    kanban_list: KanbanMarkdown.KanbanList;
    kanban_task: KanbanMarkdown.KanbanTask;
    setTaskModalState: Setter<boolean>;
};

export const KanbanTask: Component<KanbanTaskProps> = (props) => {
    const { kanban_list, kanban_task, setTaskModalState } = props;

    const [getKanbanLabels, setKanbanLabels] = createSignal<KanbanMarkdown.KanbanLabel[] | undefined>(kanban_task.labels);

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
        setTaskModalState(true);
    }

    onMount(() => {
        if (kanban_task_name_reference) {
            applyAutoResize(kanban_task_name_reference);
        }
    });

    const [getTaskMenuState, setTaskMenuState] = createSignal<boolean>(false);
    const [getTaskMenuActionsState, setTaskMenuActionsState] = createSignal<boolean>(false);

    return (
        <a class={styles.kanban_task} ref={kanban_task_name_reference} onClick={onClick}
            onMouseOver={() => {
                setTaskMenuState(true);
            }}
            onMouseLeave={() => {
                setTaskMenuState(false);
            }}>
            <span class={styles.kanban_task_title} onBlur={onBlur} onKeyPress={onKeyPress}>
                {getName()}
            </span>
            <Show when={getTaskMenuState()}>
                <button class={styles.kanban_task_menu_button}
                    onClick={(event) => {
                        event.stopPropagation();
                        setTaskMenuActionsState(b => !b);
                    }} textContent={'\u2710'}>
                </button>
            </Show>
            <Show when={getTaskMenuActionsState()}>
                <div class={styles.kanban_task_menu_actions}>
                    <button class={styles.kanban_task_menu_action_button}
                        onClick={(event) => {
                            event.stopPropagation();
                            // editCard($card, $cardTitleInput);
                            setTaskModalState(true);
                            setTaskMenuActionsState(false);
                        }} >
                        Edit
                    </button>
                    <button class={styles.kanban_task_menu_action_button}
                        onClick={(event) => {
                            event.stopPropagation();
                            // @ts-ignore
                            vscode.postMessage({
                                type: 'delete',
                                path: `list[${kanban_list.name}].tasks[${kanban_task.name}][${kanban_task.counter}]`
                            });
                            setTaskMenuActionsState(false);
                        }} >
                        Delete
                    </button>
                </div>
            </Show>
            <div class={styles.kanban_label_bar}>
                <Show when={getKanbanLabels()}>
                    <For each={getKanbanLabels()}>
                        {label => (
                            <button class={styles.kanban_label_button} style={`background-color: ${label.color};`}
                                onClick={() => {
                                    // @ts-ignore
                                    vscode.postMessage({
                                        type: 'delete',
                                        path: `list[${kanban_list.name}].tasks[${kanban_task.name}].labels[${label.name}]`
                                    });
                                }}>
                                {label.name}
                            </button>
                        )}
                    </For>
                </Show>
            </div>
        </a>
    );
}
