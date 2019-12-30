"use strict";

import * as _ from "underscore";
import { IWebappBridgeModule } from "./webappbridgemodule";

export class WebappBridge {
	public module: IWebappBridgeModule = null;

	private static instance: WebappBridge;
	static getInstance(moduleObj: any): WebappBridge {
		WebappBridge.instance = WebappBridge.instance || new WebappBridge(moduleObj);
		return WebappBridge.instance;
	}

	private _isInitialized: boolean = false;

	constructor(moduleObj: any) {
		if (WebappBridge.instance)
			throw new Error("Error - use Singleton.getInstance()");

		this.module = moduleObj;
	}

	public initialize(timeout: number = 3000): Promise<boolean> {
		if (this._isInitialized)
			return Promise.resolve(true);

		return new Promise((resolve, reject) => {
			const timeoutId = setTimeout(() => resolve(false), timeout);

			this.module.onAbort = (reason: Error | any) => {
				if (this._isInitialized) {
					// Empty
				} else {
					clearTimeout(timeoutId);
					console.log(`initializeRuntime: failed to initialize module`, reason);
					throw reason instanceof Error ? reason : new Error(reason);
				}
			}

			this.module.onRuntimeInitialized = () => {
				clearTimeout(timeoutId);
				this._isInitialized = true;
				console.log(`initializeRuntime: successfully initialized module`);
				resolve(true);
			}
		});
	}
};
