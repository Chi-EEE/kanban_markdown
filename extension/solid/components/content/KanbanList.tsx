import type { Accessor, Component, Setter } from 'solid-js';
import { Show, For, createSignal, createEffect, onMount } from "solid-js";

import styles from './KanbanList.module.css';

import { KanbanMarkdown } from '../../types';
import { KanbanTask } from './KanbanTask';
import { TemporaryKanbanTask } from './TemporaryKanbanTask';

import { applyAutoResize } from '../../utils';
import { SetStoreFunction } from 'solid-js/store';

type KanbanListProps = {
    state: KanbanMarkdown.State;
    setState: SetStoreFunction<KanbanMarkdown.State>;

    kanban_list: KanbanMarkdown.KanbanList;

    setTaskModalState: Setter<boolean>;
};

export const KanbanList: Component<KanbanListProps> = (props) => {
    const { state, setState, kanban_list, setTaskModalState } = props;

    const [getName, setName] = createSignal<string>(kanban_list.name);

    let kanban_list_name_reference: HTMLTextAreaElement | undefined;
    const [getCardTextAreaReference, setCardTextAreaReference] = createSignal<HTMLTextAreaElement>();

    createEffect(() => {
        const currentName = getName();
        const previousName = kanban_list.name;
        if (currentName !== previousName) {
            // @ts-ignore
            vscode.postMessage({
                commands: [
                    {
                        action: 'update',
                        path: `list["${encodeURI(previousName)}"].name`,
                        value: currentName
                    }
                ]
            });
            setState("kanban_board", "lists", (list, index) => list.name === previousName, {
                name: currentName,
            });
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
                <Show when={kanban_list.tasks.length > 0}>
                    <For each={kanban_list.tasks}>
                        {(kanban_task) => {
                            const kanban_task_props = {
                                state,
                                setState,
                                kanban_list,
                                kanban_task,
                                setTaskModalState,
                            }
                            return <KanbanTask {...kanban_task_props} />
                        }}
                    </For>
                </Show>
                <Show when={getAddButtonVisiblity()} fallback={
                    <TemporaryKanbanTask
                        kanban_list={kanban_list}
                        applyAutoResize={applyAutoResize}
                        setAddButtonVisiblity={setAddButtonVisiblity}
                        setCardTextAreaReference={setCardTextAreaReference}
                    />
                }>
                    <button class={styles.add_card_button} onClick={() => {
                        setAddButtonVisiblity(false);
                        getCardTextAreaReference().focus();
                    }}>Add another card +</button>
                </Show>
            </div>
        </div>
    );
}
