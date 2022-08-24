/* eslint-disable @typescript-eslint/no-explicit-any */
import ReactDOM from 'react-dom';
import { Modal } from '../modal';
import '../../style/style.css';
import '../../style/modal_style.css';

class View_Contract_Task_Index {
	public static create = (contract_task_id: number, json: string): void => {
        const Element: Modal = new Modal('view', `view_contract_task_${ contract_task_id }`, 'View Contract Task', 'modal', 'none', 'json', 'ace-modal', true, JSON.stringify(JSON.parse(json), null, '\t'), null);
		ReactDOM.render(<Element.Make/>,
		document.getElementById('root'));
	};
}

(window as any).View_Contract_Task_Index = View_Contract_Task_Index;