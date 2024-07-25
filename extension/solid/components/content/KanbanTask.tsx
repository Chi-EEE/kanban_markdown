import type { Component, Setter } from 'solid-js';
import { createSignal, createEffect, onMount, Show, For, Switch, Match } from "solid-js";

import styles from './KanbanTask.module.css';

import { KanbanMarkdown } from '../../types';

import { applyAutoResize } from '../../utils';

type KanbanTaskProps = {
    kanban_list: KanbanMarkdown.KanbanList;
    kanban_task: KanbanMarkdown.KanbanTask;
    setTaskModalState: Setter<boolean>;
    setSelectedList: Setter<KanbanMarkdown.KanbanList | undefined>;
    setSelectedTask: Setter<KanbanMarkdown.KanbanTask | undefined>;
};

export const KanbanTask: Component<KanbanTaskProps> = (props) => {
    const { kanban_list, kanban_task, setTaskModalState, setSelectedList, setSelectedTask } = props;

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
                commands: [
                    {
                        action: 'update',
                        path: `list["${encodeURI(kanban_list.name)}"].tasks["${encodeURI(previousName)}"].name`,
                        value: currentName
                    }
                ]
            });
            setPreviousName(currentName);
        }
    });

    onMount(() => {
        if (kanban_task_name_reference) {
            applyAutoResize(kanban_task_name_reference);
        }
    });

    const [getTaskMenuState, setTaskMenuState] = createSignal<boolean>(false);
    const [getTaskMenuActionsState, setTaskMenuActionsState] = createSignal<boolean>(false);

    return (
        <a class={styles.kanban_task} ref={kanban_task_name_reference}
            onClick={(event) => {
                const target = event.target as HTMLTextAreaElement;
                setSelectedList(kanban_list);
                setSelectedTask(kanban_task);
                setTaskModalState(true);
            }}
            onMouseOver={() => {
                setTaskMenuState(true);
            }}
            onMouseLeave={() => {
                setTaskMenuState(false);
            }}>
            <span class={styles.kanban_task_title}
                onBlur={(event) => {
                    const target = event.target as HTMLTextAreaElement;
                    target.value = target.value.replace(/[\v\n]+/g, '').trim();
                    setName(target.value);
                }}
                onKeyPress={(event) => {
                    const target = event.target as HTMLTextAreaElement;
                    if (event.key === 'Enter' && !event.shiftKey) {
                        target.blur();
                    } else {
                        applyAutoResize(target);
                    }
                }}
            >
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
                                commands: [
                                    {
                                        action: 'delete',
                                        path: `list["${encodeURI(kanban_list.name)}"].tasks["${encodeURI(kanban_task.name)}"][${kanban_task.counter}]`
                                    }
                                ]
                            });
                            setTaskMenuActionsState(false);
                        }} >
                        Delete
                    </button>
                </div>
            </Show>
            <Show when={getKanbanLabels() && getKanbanLabels().length > 0}>
                <div class={styles.kanban_label_bar}>
                    <For each={getKanbanLabels()}>
                        {label => (
                            <button class={styles.kanban_label_button} style={`background-color: ${label.color};`}
                                onClick={() => {
                                    // @ts-ignore
                                    vscode.postMessage({
                                        commands: [
                                            {
                                                action: 'delete',
                                                path: `list["${encodeURI(kanban_list.name)}"].tasks["${encodeURI(kanban_task.name)}"].labels["${encodeURI(label.name)}"]`
                                            }
                                        ]
                                    });
                                }}>
                                {label.name}
                            </button>
                        )}
                    </For>
                </div>
            </Show>
        </a>
    );
}
