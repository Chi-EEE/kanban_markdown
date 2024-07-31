import type { Component, Setter } from 'solid-js';
import { createSignal, createEffect, onMount, Show, For, on } from "solid-js";
import { produce, SetStoreFunction } from 'solid-js/store';

import styles from './KanbanTask.module.css';

import { KanbanMarkdown } from '../../types';

import { applyAutoResize } from '../../utils';

type KanbanTaskProps = {
    setState: SetStoreFunction<KanbanMarkdown.State>;

    kanban_list: KanbanMarkdown.KanbanList;
    kanban_task: KanbanMarkdown.KanbanTask;

    setTaskModalState: Setter<boolean>;
};

export const KanbanTask: Component<KanbanTaskProps> = (props) => {
    const {
        setState,

        kanban_list,
        kanban_task,
        
        setTaskModalState,
    } = props;

    const [getName, setName] = createSignal<string>(kanban_task.name);

    let kanban_task_name_reference: HTMLAnchorElement;

    createEffect(on(() => getName(), () => {
        const currentName = getName();
        const previousName = kanban_task.name;
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
            }); {
                setState("kanban_board", "lists", (list, index) => list.name === kanban_list.name, "tasks", (task, index) => task.name === kanban_task.name, {
                    name: currentName,
                });
            }
        }
    }));

    onMount(() => {
        if (kanban_task_name_reference) {
            applyAutoResize(kanban_task_name_reference);
        }
    });

    const [getTaskMenuState, setTaskMenuState] = createSignal<boolean>(false);
    const [getTaskMenuActionsState, setTaskMenuActionsState] = createSignal<boolean>(false);

    const setSelected = (event: MouseEvent) => {
        const target = event.target as HTMLTextAreaElement;
        setState(produce((state) => {
            state.selectedList = kanban_list;
            state.selectedTask = kanban_task;
        }));
        setTaskModalState(true);
        setTaskMenuActionsState(false);
    }

    const removeCurrentTask = (event: MouseEvent) => {
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
        setState("kanban_board", "lists", (list, index) => list.name === kanban_list.name, produce((kanban_list) => {
            kanban_list.tasks = kanban_list.tasks.filter((task) => task.name !== kanban_task.name || task.counter !== kanban_task.counter);
        }));
        setTaskMenuActionsState(false);
    }

    const removeLabel = (kanban_label: KanbanMarkdown.KanbanLabel) => {
        const labelName = kanban_label.name;
        // @ts-ignore
        vscode.postMessage({
            commands: [
                {
                    action: 'delete',
                    path: `list["${encodeURI(kanban_list.name)}"].tasks["${encodeURI(kanban_task.name)}"][${kanban_task.counter}].labels["${encodeURI(labelName)}"]`
                }
            ]
        });
        setState("kanban_board", "lists", (list, index) => list.name === kanban_list.name, "tasks", (task, index) => task.name === kanban_task.name && task.counter === kanban_task.counter, produce((kanban_task) => {
            // Only filter one label at a time
            let seenLabel = false;
            const updatedLabels = kanban_task.labels.filter((label) => {
                if (seenLabel) {
                    return true;
                }
                seenLabel = label.name === labelName;
                return !seenLabel;
            });
            kanban_task.labels = updatedLabels;
            return kanban_task;
        }));
    }

    return (
        <a class={styles.kanban_task} ref={kanban_task_name_reference}
            onClick={(event) => setSelected(event)}
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
                {kanban_task.name}
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
                        onClick={(event) => setSelected(event)} >
                        Edit
                    </button>
                    <button class={styles.kanban_task_menu_action_button}
                        onClick={(event) => removeCurrentTask(event)} >
                        Delete
                    </button>
                </div>
            </Show>
            <Show when={kanban_task.labels.length > 0}>
                <div class={styles.kanban_label_bar}>
                    <For each={kanban_task.labels}>
                        {kanban_label => (
                            <button class={styles.kanban_label_button} style={`background-color: ${kanban_label.color};`}
                                onClick={() => removeLabel(kanban_label)}>
                                {kanban_label.name}
                            </button>
                        )}
                    </For>
                </div>
            </Show>
        </a>
    );
}
