// @flow
import { mapFor } from '../Utils/MapFor';
import { type Schema, type Instance } from '.';

/**
 * Transform a MapStringPropertyDescriptor to a schema that can be used in PropertiesEditor.
 *
 * @param {gdMapStringPropertyDescriptor} properties The properties to use
 * @param {*} getProperties The function called to read again the properties
 * @param {*} onUpdateProperty The function called to update a property of an object
 */
export default (
  properties: gdMapStringPropertyDescriptor,
  getProperties: (instance: Instance) => any,
  onUpdateProperty: (
    instance: Instance,
    propertyName: string,
    newValue: string
  ) => void
): Schema => {
  const propertyNames = properties.keys();
  const propertyFields = mapFor(0, propertyNames.size(), i => {
    const name = propertyNames.at(i);
    const property = properties.get(name);
    const valueType = property.getType().toLowerCase();
    const choices = property
      .getExtraInfo()
      .toJSArray()
      .map(value => ({ value, label: value }));

    // $FlowFixMe
    return {
      name,
      valueType,
      getChoices: valueType === 'choice' ? () => choices : undefined,
      getValue: instance => {
        // Instance custom properties are always stored as string, cast them if necessary
        const rawValue = getProperties(instance)
          .get(name)
          .getValue();
        if (valueType === 'boolean') {
          return rawValue === 'true';
        } else if (valueType === 'number') {
          return parseFloat(rawValue);
        }

        return rawValue;
      },
      setValue: (instance, newValue) => {
        // Instance custom properties are always stored as string, cast them if necessary
        let value;
        if (typeof newValue === 'boolean') {
          value = newValue ? '1' : '0';
        } else {
          value = '' + newValue;
        }

        onUpdateProperty(instance, name, value);
      },
      getLabel: instance => {
        const propertyName = getProperties(instance)
          .get(name)
          .getLabel();
        if (propertyName) return propertyName;
        return (
          name.charAt(0).toUpperCase() +
          name
            .slice(1)
            .split(/(?=[A-Z])/)
            .join(' ')
        );
      },
    };
  });

  return propertyFields;
};
