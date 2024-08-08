import { date } from 'quasar';

export const formatTimestamp  = (dateInput: Date): string => {
    return date.formatDate(dateInput, 'YYYYMMDD_HHmmss');
};