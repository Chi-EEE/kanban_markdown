import type { Component } from 'solid-js';
import { Show, For, createSignal, createEffect, onMount } from "solid-js";

import styles from './KanbanList.module.css';

import { KanbanMarkdown } from '../../types';
import { KanbanTask } from './KanbanTask';

import { applyAutoResize } from '../../utils';

type KanbanListProps = {
    kanban_list: KanbanMarkdown.KanbanList;
    setTaskModalState: (state: boolean) => void;
};

export const KanbanList: Component<KanbanListProps> = (props) => {
    const { kanban_list, setTaskModalState } = props;

    const [getName, setName] = createSignal<string>(kanban_list.name);
    const [getPreviousName, setPreviousName] = createSignal<string>(kanban_list.name);

    const [getKanbanListTasks, setKanbanListTasks] = createSignal<KanbanMarkdown.KanbanTask[]>(kanban_list.tasks);

    let kanban_list_name_reference: HTMLTextAreaElement | undefined;
    let card_textarea_reference: HTMLTextAreaElement | undefined;

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

    const [getAddButtonVisiblity, setAddButtonVisiblity] = createSignal<boolean>(true);

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
            <div class={styles.kanban_task_list}>
                <Show when={getKanbanListTasks()}>
                    <For each={getKanbanListTasks()}>
                        {(kanban_task, index) => (
                            <KanbanTask kanban_list={kanban_list} kanban_task={kanban_task} setTaskModalState={setTaskModalState} />
                        )}
                    </For>
                </Show>
                <Show when={getAddButtonVisiblity()} fallback={
                    <a class={styles.temp_card}>
                        <textarea
                            class={styles.temp_card_title}
                            ref={card_textarea_reference}
                            placeholder='Enter card title'
                            onBlur={(event: FocusEvent) => {
                                const target = event.target as HTMLTextAreaElement;
                                target.value = target.value.replace(/[\v\n]+/g, '').trim();
                                if (target.value !== '') {
                                    const new_task: KanbanMarkdown.KanbanTask = {
                                        name: target.value,
                                        checked: false,
                                        description: '',
                                        labels: [],
                                        attachments: [],
                                        checklist: [],
                                        counter: 0,
                                    };
                                    // @ts-ignore
                                    vscode.postMessage({
                                        type: 'create',
                                        path: `list[${getPreviousName()}].tasks`,
                                        value: new_task
                                    });
                                    setKanbanListTasks([...getKanbanListTasks(), new_task]);
                                }
                                target.value = '';
                                setAddButtonVisiblity(true);
                            }}
                            onKeyPress={(event: KeyboardEvent) => {
                                const target = event.target as HTMLTextAreaElement;
                                if (event.key === 'Enter' && !event.shiftKey) {
                                    target.blur();
                                } else {
                                    applyAutoResize(target);
                                }
                            }}
                        >

                        </textarea>
                    </a>
                }>
                    <button class={styles.add_card_button} onClick={() => {
                        setAddButtonVisiblity(false);
                        card_textarea_reference.focus();
                    }}>Add another card +</button>
                </Show>
            </div>
        </div>
    );
}
