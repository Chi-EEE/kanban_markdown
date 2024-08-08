import type { Accessor, Component, JSX, JSXElement, Setter } from 'solid-js';
import { Show, For, createSignal, createEffect, onMount, on, children } from "solid-js";

import styles from './KanbanList.module.css';

import { KanbanMarkdown } from '../../types';
import { TemporaryKanbanTask } from './TemporaryKanbanTask';

import { applyAutoResize } from '../../utils';
import { produce, SetStoreFunction } from 'solid-js/store';
import { createSortable, maybeTransformStyle } from '@thisbeyond/solid-dnd';

type KanbanListProps = {
    children: JSXElement;

    state: KanbanMarkdown.State;
    setState: SetStoreFunction<KanbanMarkdown.State>;

    kanban_list: KanbanMarkdown.KanbanList;
};

export const KanbanList: Component<KanbanListProps> = (props) => {
    const {
        state,
        setState,

        kanban_list,
    } = props;

    const [getName, setName] = createSignal<string>(kanban_list.name);

    let kanban_list_name_reference: HTMLTextAreaElement | undefined;
    const [getCardTextAreaReference, setCardTextAreaReference] = createSignal<HTMLTextAreaElement>();

    createEffect(on(() => getName(), (currentName) => {
        const previousName = kanban_list.name;
        if (currentName !== previousName) {
            setState("kanban_board", "lists", (list, index) => list.name === previousName && list.counter === kanban_list.counter,
                produce((kanban_list) => {
                    kanban_list.name = currentName;
                    state.kanban_board.list_name_tracker_map.get(previousName).removeHash(kanban_list.counter);
                    kanban_list.counter = KanbanMarkdown.DuplicateNameTracker.GetCounterWithName(currentName, state.kanban_board.list_name_tracker_map);
                }));
            // @ts-ignore
            vscode.postMessage({
                commands: [
                    {
                        action: 'update',
                        path: `list["${encodeURI(previousName)}"][${kanban_list.counter}].name`,
                        value: currentName
                    }
                ]
            });
        }
    }));

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

    const sortable = createSortable(`list-${kanban_list.name}-${kanban_list.counter}`, {
        name: kanban_list.name,
        counter: kanban_list.counter,
        type: "list",
    });

    return (
        <div
            ref={sortable.ref}
            style={{ ...maybeTransformStyle(sortable.transform), opacity: sortable.isActiveDraggable ? 0.25 : 1 }}
            class={styles.kanban_list}
        >
            <textarea
                {...sortable.dragActivators}
                ref={kanban_list_name_reference}
                class={styles.kanban_list_name}
                placeholder='Enter list name'
                onBlur={onBlur}
                onKeyPress={onKeyPress}
            >
                {kanban_list.name}
            </textarea>
            <div class={styles.kanban_task_list}>
                {props.children}
                <Show when={getAddButtonVisiblity()} fallback={
                    <TemporaryKanbanTask
                        state={state}
                        setState={setState}
                        kanban_list={kanban_list}
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
