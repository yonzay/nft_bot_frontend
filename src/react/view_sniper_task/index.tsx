/* eslint-disable @typescript-eslint/no-explicit-any */
import ReactDOM from 'react-dom';
import { Modal } from '../modal';
import '../../style/style.css';
import '../../style/modal_style.css';

class View_Sniper_Task_Index {
	public static create = (sniper_task_id: string, json: string): void => {
        const Element: Modal = new Modal('view', `view_sniper_task_${ sniper_task_id }`, 'View Sniper Task', 'modal', 'none', 'json', 'ace-modal', true, JSON.stringify(JSON.parse(json), null, '\t'), null);
		ReactDOM.render(<Element.Make/>,
		document.getElementById('root'));
	};
}

(window as any).View_Sniper_Task_Index = View_Sniper_Task_Index;