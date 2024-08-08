import type { Accessor, Setter, VoidComponent } from 'solid-js';
import { createSignal, createEffect, onMount, Show, For, on } from "solid-js";
import { produce, SetStoreFunction } from 'solid-js/store';

import styles from './KanbanTask.module.css';

import { KanbanMarkdown } from '../../types';

import { applyAutoResize } from '../../utils';
import { createSortable } from '@thisbeyond/solid-dnd';

type KanbanTaskProps = {
    state: KanbanMarkdown.State;
    setState: SetStoreFunction<KanbanMarkdown.State>;

    kanban_list: KanbanMarkdown.KanbanList;
    kanban_task: KanbanMarkdown.KanbanTask;

    draggingState: Accessor<boolean>;
    setTaskModalState: Setter<boolean>;
};

export const KanbanTask: VoidComponent<KanbanTaskProps> = (props) => {
    const {
        state,
        setState,

        kanban_list,
        kanban_task,

        draggingState,
        setTaskModalState,
    } = props;

    const [getName, setName] = createSignal<string>(kanban_task.name);

    let kanban_task_name_reference: HTMLAnchorElement;

    createEffect(on(() => getName(), (currentName) => {
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
                setState("kanban_board", "lists", (list, index) => list.name === kanban_list.name && list.counter === kanban_list.counter, "tasks", (task, index) => task.name === kanban_task.name,
                    produce((kanban_task) => {
                        kanban_task.name = currentName;
                        state.kanban_board.task_name_tracker_map.get(previousName).removeHash(kanban_task.counter);
                        kanban_task.counter = KanbanMarkdown.DuplicateNameTracker.GetCounterWithName(currentName, state.kanban_board.task_name_tracker_map);
                    }));
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
        setState(produce((state) => {
            const kanban_list: KanbanMarkdown.KanbanList = state.kanban_board.lists.find((list) => list.name === kanban_list.name && list.counter === kanban_list.counter);
            const kanban_task: KanbanMarkdown.KanbanTask = kanban_list.tasks.find((task) => task.name === kanban_task.name && task.counter === kanban_task.counter);
            state.kanban_board.task_name_tracker_map.get(kanban_task.name).removeHash(kanban_task.counter);
            const index = kanban_list.tasks.indexOf(kanban_task);
            kanban_list.tasks.splice(index, 1);
            return state;
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
                    path: `list["${encodeURI(kanban_list.name)}"][${kanban_list.counter}].tasks["${encodeURI(kanban_task.name)}"][${kanban_task.counter}].labels["${encodeURI(labelName)}"]`
                }
            ]
        });
        setState("kanban_board", "lists", (list, index) => list.name === kanban_list.name && list.counter === kanban_list.counter, "tasks", (task, index) => task.name === kanban_task.name && task.counter === kanban_task.counter, produce((kanban_task) => {
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

    const sortable = createSortable(`task-${kanban_task.name}-${kanban_task.counter}`, {
        name: kanban_task.name,
        counter: kanban_task.counter,
        type: "task",
        list: `list-${kanban_list.name}-${kanban_list.counter}`,
    });

    createEffect(on(() => draggingState(), (state) => {
        if (state) {
            setTaskMenuState(false);
        }
    }));

    return (
        <a
            // @ts-ignore
            use:sortable
            style={{ opacity: sortable.isActiveDraggable ? 0.25 : 1 }}
            class={styles.kanban_task}
            ref={kanban_task_name_reference}
            onClick={(event) => {
                if (getTaskMenuState())
                    setSelected(event);
            }}
            onMouseOver={() => {
                if (!draggingState())
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
        </a >
    );
}
