import styles from '../TaskModal.module.css';

import { createSignal, For, Show } from "solid-js";
import type { Accessor, Component, Setter } from 'solid-js';
import { KanbanMarkdown } from "../../../types";
import { produce, SetStoreFunction, unwrap } from 'solid-js/store';

type LabelMenuProps = {
    state: KanbanMarkdown.State
    setState: SetStoreFunction<KanbanMarkdown.State>;

    setLabelMenuReference: Setter<HTMLDivElement>;
};

export const LabelMenu: Component<LabelMenuProps> = (props) => {
    const { state, setState, setLabelMenuReference } = props;

    const [getLabelMenuState, setLabelMenuState] = createSignal<string>("select");

    let modal_new_label_name_input_reference: HTMLInputElement;
    let modal_new_label_color_reference: HTMLInputElement;

    const addLabelToSelectedTask = (kanban_label: KanbanMarkdown.KanbanLabel) => {
        const selectedList = state.selectedList;
        const selectedTask = state.selectedTask;
        // @ts-ignore
        vscode.postMessage({
            commands: [
                {
                    action: 'create',
                    path: `list["${encodeURI(selectedList.name)}"][${selectedList.counter}].tasks["${encodeURI(selectedTask.name)}"][${selectedTask.counter}].labels`,
                    value: unwrap(kanban_label),
                }
            ]
        });
        setState(produce((state) => {
            const kanban_task = state.kanban_board.lists
                .find((list) => list.name === selectedList.name && list.counter === selectedList.counter).tasks
                .find((task) => task.name === selectedTask.name && task.counter === selectedTask.counter);
            kanban_task.labels.push(kanban_label);
        }));
    }

    const createLabel = () => {
        const name = modal_new_label_name_input_reference.value;
        const color = modal_new_label_color_reference.value;
        const selectedList = state.selectedList;
        const selectedTask = state.selectedTask;
        // @ts-ignore
        vscode.postMessage({
            commands: [
                {
                    action: 'create',
                    path: `labels`,
                    value: {
                        name: name,
                        color: color,
                    }
                },
                {
                    action: 'create',
                    path: `list["${encodeURI(selectedList.name)}"][${selectedList.counter}].tasks["${encodeURI(selectedTask.name)}"][${selectedTask.counter}].labels`,
                    value: {
                        name: name,
                        color: color,
                    }
                }
            ]
        });
        setState(produce((state) => {
            const kanban_label = {
                name: name,
                color: color,
                tasks: []
            };

            state.kanban_board.labels.push(kanban_label);
            const kanban_task = state.kanban_board.lists
                .find((list) => list.name === selectedList.name && list.counter === selectedList.counter).tasks
                .find((task) => task.name === selectedTask.name && task.counter === selectedTask.counter);
            kanban_task.labels.push(kanban_label);
        }));
        setLabelMenuState("select");
    }

    return (
        <div ref={(el) => setLabelMenuReference(el)} class={styles.menu}>
            <Show when={getLabelMenuState() === "select"}>
                <div>
                    <h3>Labels</h3>
                    <div id={styles.modal_label_list}>
                        <For each={state.kanban_board.labels}>
                            {(kanban_label) => (
                                <div class={styles.modal_label} style={`background-color: ${kanban_label.color}`}
                                    onClick={() => addLabelToSelectedTask(kanban_label)}>
                                    {kanban_label.name}
                                </div>
                            )}
                        </For>
                    </div>
                    <button
                        onClick={() => setLabelMenuState("create")}>Create a new label</button>
                </div>
            </Show>
            <Show when={getLabelMenuState() === "create"}>
                <div>
                    <h3>Create Label</h3>
                    <label for="modal-new-label-title">Title</label>
                    <input ref={modal_new_label_name_input_reference} type="text" id="modal-new-label-title" />
                    <label for="modal-new-label-color">Color</label>
                    <input ref={modal_new_label_color_reference} type="color" id="modal-new-label-color" value="#ffffff" />
                    <button id="modal-create-label"
                        onClick={() => createLabel()}>
                        Create
                    </button>
                    <button id="modal-back-to-label-select"
                        onClick={() => setLabelMenuState("select")}>Back</button>
                </div>
            </Show>
        </div>
    );
}
