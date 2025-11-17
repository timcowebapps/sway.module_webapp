"use strict";

export class Application {
	public wasm_module: any = null;

	constructor(public wasm_context: any = null, elementId: string) {
		this.wasm_module = new wasm_context.module.Application(elementId);
	}
};
